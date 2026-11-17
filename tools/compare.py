#!/usr/bin/env python3
"""Compare two identity arms (or two slots of one arm) state by state, and print what ran where.

  python3 tools/compare.py <armA>[:slot] <armB>[:slot] [MODE=records|search] [EXPECT=same|differ]
     arm = a folder holding result.json and dec/ (one run of the bot over the frozen positions); slot defaults to 0.

MODE=records (default): for every state, the move played, the legacy decision record (dec_*.bin) and the full decision
  record (decfull_*.dxf) must be BIT FOR BIT identical, except these fields, which are not decisions:
    legacy: ts_ns, handle, slot; the counter stage's ms_start / ms_used (XPC1)
    full:   ts_ns, handle, slot, pid; META ms_final, ms_write, capture_us, build_us, lp_ms (the planner's wall time); every CALL row's ms;
            KOST ms_start .. ms_stage and dispatchers (the worker count: arms may run on different CPU counts); OPEN ms1, ms2. The separate time / commit records are not compared.
MODE=search: what DECAP_EXPLAIN=1 adds is left out too (the explanation trailer and the legacy size field that counts it,
  META flag 16 and the REPS rows'
  source byte): the move, the legacy candidate list, and the full record's META (but flag 16), CAND, FINL, PLAN, KOST,
  OPEN and the REPS joints / sets must be identical. Use it for "log on" against "log off".
Exit status 0 when the result is what EXPECT says (default same), 1 otherwise.
"""
import sys, os, json, glob, struct, collections
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dec_read as R

ARGS = [a for a in sys.argv[1:] if "=" not in a]
OPT = dict(a.split("=", 1) for a in sys.argv[1:] if "=" in a)
MODE = OPT.get("MODE", "records")
EXPECT = OPT.get("EXPECT", "same")


def arm(spec):
    name, _, slot = spec.partition(":")
    d = name
    return d, json.load(open(os.path.join(d, "result.json"))), int(slot or 0)


def zero(b, o, n):
    b[o:o + n] = b"\0" * n


# ---- legacy record (dec_*.bin) ------------------------------------------------------------------------------------------
def legacy(dec):
    """(tag, slot) -> [record bytes]"""
    out = collections.defaultdict(list)
    for f in sorted(glob.glob(dec + "/dec_*.bin")):
        data = open(f, "rb").read()
        o = 0
        while o + R.HDR.size <= len(data):
            h = R.HDR.unpack_from(data, o)
            if h[0] != R.MAGIC or h[1] < R.HDR.size or o + h[1] > len(data):
                break
            out[(h[3], h[6])].append(data[o:o + h[1]])
            o += h[1]
    return out


def legacy_view(rec, mode):
    b = bytearray(rec)
    h = R.HDR.unpack_from(b, 0)
    zero(b, 8, 8); zero(b, 20, 4); zero(b, 28, 4)  # ts_ns, handle, slot
    p = R.HDR.size + h[12] + h[8] * h[13]  # req_size, n_cand x cand_size
    p += R.RESP.size
    end = len(b)
    if mode == "search":
        zero(b, 4, 4)  # (the record's size: it includes the explanation trailer)
        return bytes(b[:p])
    if p + 12 <= end and struct.unpack_from("<I", b, p)[0] == R.XP_MAGIC:
        _, n_rep, n_rows, _, _ = struct.unpack_from("<IHHHH", b, p)
        q = p + 12 + n_rep * R.XP_REP.size
        for _ in range(n_rows):
            n = R.XP_ROW.unpack_from(b, q)[3]
            q += R.XP_ROW.size + n * R.XP_OUT.size
        if q + R.XP_CS.size <= end and struct.unpack_from("<I", b, q)[0] == R.XP_CS_MAGIC:
            zero(b, q + 20, 8)  # ms_start, ms_used
    return bytes(b)


# ---- full record (decfull_*.dxf) ----------------------------------------------------------------------------------------
def offsets(fmt, keys):
    o, out, i = 0, {}, 0
    import re
    for cnt, c in re.findall(r"(\d*)([a-zA-Z])", fmt[1:]):
        n = int(cnt or 1)
        if c in "sx":
            if c == "s":
                out[keys[i]] = o; i += 1
            o += n
            continue
        for _ in range(n):
            out[keys[i]] = o; i += 1
            o += struct.calcsize("<" + c)
    return out


META_OFF = offsets(R.FX_META.format if isinstance(R.FX_META.format, str) else R.FX_META.format.decode(), R.FX_META_KEYS)
KO_KEYS = R.FX_KO_KEYS
KO_OFF = offsets(R.FX_KO.format if isinstance(R.FX_KO.format, str) else R.FX_KO.format.decode(), KO_KEYS)
OPEN_OFF = offsets(R.FX_OPEN.format if isinstance(R.FX_OPEN.format, str) else R.FX_OPEN.format.decode(), R.FX_OPEN_KEYS)


def full(dec):
    out = collections.defaultdict(list)
    for f in sorted(glob.glob(dec + "/decfull_*.dxf")):
        data = open(f, "rb").read()
        o = 0
        while o + 8 <= len(data):
            magic, size = struct.unpack_from("<II", data, o)
            if size < 8 or o + size > len(data):
                break
            if magic == R.FX_MAGIC:
                h = R.FX_HDR.unpack_from(data, o)
                out[(h[6], h[9])].append(data[o:o + size])
            o += size
    return out


def full_view(rec, mode):
    """-> {section name: bytes} with the non-decision fields zeroed"""
    b = bytearray(rec)
    h = R.FX_HDR.unpack_from(b, 0)
    for off in (16, 28, 36, 44):  # ts_ns (8 bytes at 16), handle, slot, pid
        zero(b, off, 8 if off == 16 else 4)
    secs = collections.OrderedDict(HDR=bytes(b[:h[3]]))
    p, end = h[3], len(b) - 4
    while p + R.FX_SEC.size <= end:
        sid, n = R.FX_SEC.unpack_from(b, p)
        q = p + R.FX_SEC.size
        name = bytes(b[q - 8:q - 4]).decode("latin-1")
        if name == "META":
            for k in ("ms_final", "ms_write", "capture_us", "build_us", "lp_ms"):
                zero(b, q + META_OFF[k], 4)
            if mode == "search":
                b[q + META_OFF["flags"]] &= ~16 & 0xFF
        elif name in ("CALL", "REPS"):
            row, _, cnt = R.FX_ROWS.unpack_from(b, q)
            for i in range(cnt):
                r0 = q + R.FX_ROWS.size + i * row
                if name == "CALL":
                    zero(b, r0 + 12, 4)
                elif mode == "search":
                    b[r0 + 22] = 0  # (the reply's source: known only with DECAP_EXPLAIN=1)
        elif name == "KOST":
            for k in ("ms_start", "ms_end", "ms_r1", "ms_r2", "ms_safe", "ms_stage", "dispatchers"):  # dispatchers = worker count of the run
                zero(b, q + KO_OFF[k], 4)
        elif name == "OPEN":
            for k in ("ms1", "ms2"):
                zero(b, q + OPEN_OFF[k], 4)
        secs[name + ("" if name not in secs else "#%d" % p)] = bytes(b[q:q + n])
        p = q + n
    return secs


def main():
    (da, ra, sa), (db, rb, sb) = arm(ARGS[0]), arm(ARGS[1])
    La, Fa, Lb, Fb = legacy(da + "/dec"), full(da + "/dec"), legacy(db + "/dec"), full(db + "/dec")
    print("A = %s slot %d (%s md5 %s)" % (da, sa, ra["bin"], ra["md5"]))
    print("B = %s slot %d (%s md5 %s)" % (db, sb, rb["bin"], rb["md5"]))
    print("mode %s; masked fields: see the header of compare.py" % MODE)
    n_same = n_diff = n_skip = 0
    rows = []
    for sid, xa in ra["res"].items():
        xb = rb["res"].get(sid)
        if not xb or not xa.get("replay_ok") or not xb.get("replay_ok"):
            n_skip += 1; rows.append((sid, "not compared (missing or replay failed)")); continue
        why = []
        if xa["state_md5"] != xb["state_md5"]: why.append("start state")
        if xa["moves"][sa] != xb["moves"][sb]: why.append("MOVE %s vs %s" % (xa["moves"][sa], xb["moves"][sb]))
        tag = xa["tag"]
        la, fa = La.get((tag, sa), []), Fa.get((tag, sa), [])
        lb, fb = Lb.get((tag, sb), []), Fb.get((tag, sb), [])
        if len(la) != len(lb) or not la: why.append("legacy records %d vs %d" % (len(la), len(lb)))
        else:
            for i, (x, y) in enumerate(zip(la, lb)):
                if legacy_view(x, MODE) != legacy_view(y, MODE): why.append("legacy record %d" % i)
        if len(fa) != len(fb) or not fa: why.append("full records %d vs %d" % (len(fa), len(fb)))
        else:
            for i, (x, y) in enumerate(zip(fa, fb)):
                va, vb = full_view(x, MODE), full_view(y, MODE)
                bad = [k for k in sorted(set(va) | set(vb)) if va.get(k) != vb.get(k)]
                if bad: why.append("full record %d sections %s" % (i, ",".join(bad)))
        if why: n_diff += 1
        else: n_same += 1
        rows.append((sid, "IDENTICAL" if not why else "DIFFERENT: " + "; ".join(why)))
    for sid, s in rows:
        print("  %-26s %s" % (sid, s))
    print("states compared %d: identical %d, different %d; not compared %d" % (n_same + n_diff, n_same, n_diff, n_skip))
    coverage(da, ra, sa, Fa)
    ok = (n_diff == 0 and n_same > 0) if EXPECT == "same" else (n_diff > 0)
    print("VERDICT: %s (expected %s)" % ("as expected" if ok else "NOT as expected", EXPECT))
    sys.exit(0 if ok else 1)


STAGES = [("opening search (stage 10)", lambda m, d: m["final_stage"] == 10), ("book draw at turn 0 (11)", lambda m, d: m["final_stage"] == 11),
          ("held-out replies", lambda m, d: m["cem_note"] & 2 and m["final_stage"] < 10), ("adversarial replies", lambda m, d: m["cem_note"] & 4 and m["final_stage"] < 10),
          ("counter search", lambda m, d: m["cem_note"] & 8 and m["final_stage"] < 10), ("PICK1 one-turn term", lambda m, d: m["cem_note"] & 32 and m["final_stage"] < 10),
          ("launch planner (planned launches)", lambda m, d: m["cem_note"] & 64 and m["final_stage"] < 10),
          ("shadow (chase) planner", lambda m, d: m["cem_note"] & 256 and m["final_stage"] < 10),
          ("knockout ran", lambda m, d: m["flags"] & 2), ("knockout changed the move", lambda m, d: m["final_stage"] == 5),
          ("rescue (nothing safe)", lambda m, d: m["final_stage"] == 3), ("final pick over finalists (4)", lambda m, d: m["final_stage"] == 4),
          ("full decision log record", lambda m, d: True)]


def coverage(d, r, slot, F):
    cnt = collections.Counter()
    per = {}
    for sid, x in r["res"].items():
        if not x.get("replay_ok"): continue
        recs = F.get((x["tag"], slot), [])
        if not recs: continue
        m = R._fx_decision(recs[0], 0, len(recs[0]))["meta"]
        if not m: continue
        ran = [name for name, f in STAGES if f(m, None)]
        per[sid] = ran
        for name in ran: cnt[name] += 1
    print("coverage (arm %s slot %d, %d states with a full record): stage -> states it ran in" % (os.path.basename(d), slot, len(per)))
    for name, _ in STAGES:
        print("  %-36s %3d" % (name, cnt[name]))
    by = collections.defaultdict(collections.Counter)
    for sid, ran in per.items():
        for name in ran: by[sid.split(":")[0]][name] += 1
    print("  by state kind: " + "; ".join("%s: %s" % (k, ", ".join("%s %d" % (n.split(" (")[0], c) for n, c in v.items() if n != "full decision log record"))
                                         for k, v in sorted(by.items())))


if __name__ == "__main__":
    main()
