# Reads the bot's decision log (the bot's main file decap_decision_log: DECAP_DECISION_LOG=<dir>, one dec_<time>_<pid>.bin per
# process). A record: header, the network's inputs (the position block, raw), the scored candidates, the move played.
#   python dec_read.py <dir> [games json ...]   -> counts and a sample; with games json files, joins results by game id
# As a module: records(path) yields dicts; request(raw) decodes the Request into numpy arrays.
import glob, json, os, struct, sys
import numpy as np

HDR = struct.Struct("<IIQ11i2f4x")  # (72 bytes: the C++ struct is padded to 8) magic, size, ts_ns, tag, handle, player, slot, turn, n_cand, played, cem_note, ready,
#                                   req_size, cand_size, score, d0
CAND = struct.Struct("<20s2sBBfff")  # joints, grip, safe, tag, mean_v, min_v, fscore
RESP = struct.Struct("<20s2s")
MAGIC = 0x43455244
N_PARTS, N_JOINTS = 21, 20
# [REDACTED: the layout of the position block inside a record]
REQ_SIZE = None



def _skip(ts_ns):
    import os
    win = os.environ.get("DEC_SKIP", "")
    for w in win.split(","):
        if "-" in w:
            a, b = w.split("-")
            if float(a) * 1e9 <= ts_ns <= float(b) * 1e9:
                return True
    return False

def pose(raw):
    raise NotImplementedError("[REDACTED: decodes one fighter's block of the position]")


def request(raw):
    raise NotImplementedError("[REDACTED: decodes the position block of a record]")


# B15 (DECAP_EXPLAIN=1) trailer after the move played: what the search predicted (the bot's main file decap_xp_trailer)
XP_MAGIC = 0x314C5058
XP_REP = struct.Struct("<20s2sBx")       # a guessed opponent reply: joints, grip, src
XP_ROW = struct.Struct("<20s2sBB")       # a move explained: joints, grip, role, n_out
XP_OUT = struct.Struct("<HBBbb2x6f")     # one (move, reply) plan: rep, flags, call, dm_op1, dm_me1, v, value, dealt1, taken1, dealt, taken
XP_SRC = {0: "another guess", 1: "you keep your joints", 2: "a move my network thought you might make",
          3: "what top players did from your position", 4: "my network's top pick for each of your joints",
          5: "one of your strongest counters per my network", 6: "a throw or push players used from your position",
          7: "a counter my search found against this move"}  # (7: B21 DECAP_OPP_CEM)
# B21 (DECAP_OPP_CEM): the counter stage's summary block after the XPL1 rows ('XPC1', the bot's main file decap_xp_counter)
XP_CS_MAGIC = 0x31435058
XP_CS = struct.Struct("<IhhHHHHHHii8f")
XP_CS_KEYS = ("magic", "status", "rounds", "attacked", "turn_sims", "full_plans", "answered", "kept", "changed", "ms_start",
              "ms_used", "d0", "guess_taken", "counter_taken", "guess_j", "counter_j", "fs_before", "fs_after", "mean_delta")
XP_ROLE = {0: "played", 1: "keep still", 2: "network first idea", 3: "finalist", 4: "strong alternative"}


def _explain(data, p, end):
    if p + 12 > end:
        return None
    magic, n_rep, n_rows, n_tried, n_unsafe = struct.unpack_from("<IHHHH", data, p)
    if magic != XP_MAGIC:
        return None
    p += 12
    reps = []
    for _ in range(n_rep):
        j, g, src = XP_REP.unpack_from(data, p)
        reps.append(dict(joints=[int(x) for x in j], grip=[int(x) for x in g], src=src))
        p += XP_REP.size
    rows = []
    for _ in range(n_rows):
        j, g, role, n = XP_ROW.unpack_from(data, p)
        p += XP_ROW.size
        outs = []
        for _ in range(n):
            rep, flags, call, dmo, dmm, v, value, dealt1, taken1, dealt, taken = XP_OUT.unpack_from(data, p)
            outs.append(dict(rep=rep, flags=flags, call=call, dm_op1=dmo, dm_me1=dmm, v=v, value=value,
                             dealt1=dealt1, taken1=taken1, dealt=dealt, taken=taken))
            p += XP_OUT.size
        rows.append(dict(joints=[int(x) for x in j], grip=[int(x) for x in g], role=role, outs=outs))
    counter = None  # (B21: the counter stage's summary, when DECAP_OPP_CEM ran; None otherwise)
    if p + XP_CS.size <= end and struct.unpack_from("<I", data, p)[0] == XP_CS_MAGIC:
        counter = dict(zip(XP_CS_KEYS, XP_CS.unpack_from(data, p)))
    return dict(reps=reps, rows=rows, n_tried=n_tried, n_unsafe=n_unsafe, counter=counter)


def records(path, decode=False):
    data = open(path, "rb").read()
    o = 0
    while o + HDR.size <= len(data):
        h = HDR.unpack_from(data, o)
        magic, size = h[0], h[1]
        if magic != MAGIC or size < HDR.size or o + size > len(data):
            break  # (a torn record at the end of a file that was being written)
        keys = ("magic", "size", "ts_ns", "tag", "handle", "player", "slot", "turn", "n_cand", "played", "cem_note",
                "ready", "req_size", "cand_size", "score", "d0")
        r = dict(zip(keys, h))
        p = o + HDR.size
        raw_req = data[p:p + r["req_size"]]
        p += r["req_size"]
        cands = []
        for _ in range(r["n_cand"]):
            j, g, safe, tag, mean_v, min_v, fscore = CAND.unpack_from(data, p)
            cands.append(dict(joints=np.frombuffer(j, dtype=np.uint8).copy(), grip=np.frombuffer(g, dtype=np.uint8).copy(),
                              safe=safe, tag=tag, mean_v=mean_v, min_v=min_v, fscore=fscore))
            p += r["cand_size"]
        j, g = RESP.unpack_from(data, p)
        r["played_move"] = np.frombuffer(j, dtype=np.uint8).copy()
        r["cands"] = cands
        r["explain"] = _explain(data, p + RESP.size, o + size)  # (B15 DECAP_EXPLAIN trailer, else None)
        r["request"] = request(raw_req) if decode and r["req_size"] == REQ_SIZE else raw_req
        if _skip(r["ts_ns"]):  # DEC_SKIP="<from epoch s>-<to epoch s>[,...]": decisions from a bad build to leave out of training
            continue
        yield r
        o += size


def main(d, games_files):
    files = sorted(glob.glob(os.path.join(d, "dec_*.bin")))
    n = ncand = 0
    sample = None
    bad = 0
    for f in files:
        for r in records(f, decode=False):
            n += 1
            ncand += r["n_cand"]
            if r["req_size"] != REQ_SIZE:
                bad += 1
            if sample is None and r["n_cand"] > 0:
                sample = r
    print("%d files, %d records, %.1f candidates/decision, %d with an unexpected request size (expected %d)" % (
        len(files), n, ncand / max(n, 1), bad, REQ_SIZE))
    if sample:
        req = request(sample["request"]) if sample["req_size"] == REQ_SIZE else None
        print("sample: tag %d handle %d player %d slot %d turn %d played %d/%d cem_note %d score %.3f d0 %.2f" % (
            sample["tag"], sample["handle"], sample["player"], sample["slot"], sample["turn"], sample["played"],
            sample["n_cand"], sample["cem_note"], sample["score"], sample["d0"]))
        c = sample["cands"][min(sample["played"], sample["n_cand"] - 1)]
        print("  played: joints %s safe %d tag %d mean %.3f min %.3f fscore %.3f" % (
            "".join(str(int(x)) for x in sample["played_move"]), c["safe"], c["tag"], c["mean_v"], c["min_v"], c["fscore"]))
    for gf in games_files:
        g = json.load(open(gf))
        res = {int(k): v for k, v in g["games"].items()}
        joined = sum(1 for f in files for r in records(f) if g["t0"] * 1e9 <= r["ts_ns"] <= g["t_end"] * 1e9 and r["tag"] in res)
        print("%s: %d games, %d decisions in its time window with a known game" % (os.path.basename(gf), len(res), joined))


# ---- B36 full decision log (DECAP_LOG_FULL=1) ------------------------------------------------------------------------
# The side file decfull_<sec>_<pid>.dxf next to dec_<sec>_<pid>.bin (the bot's main file, decap_logfull.inc): one 'DXF1' record
# per decision, keyed by the legacy record's own ts_ns / tag / handle / player / turn. Nothing above this block changed;
# records() never opens a .dxf file.
#   full_path(legacy_path)        -> the side file's path for a dec_*.bin file
#   full_records(path)            -> every record of one .dxf file as a dict: kind "decision", "commit" or "time"
#   full_decisions(path)          -> the decisions only, each with its "commit" note and "time" record attached when present
#   full_index(dir_or_file)       -> {(ts_ns, tag, handle, player, turn): decision}
#   full_record(r, legacy_path)   -> the full record of legacy record r (a dict from records()), or None when there is none
#   finalists_table(d)            -> one row per finalist with every part of its score (see FX_PARTS)
#   decided_by(d)                 -> which stage produced the move, and the stage after which it led the finalists
#   final_pick(d, scores=None)    -> the finalist the bot's own pick rule chooses from these scores with the decision's own draw
FX_MAGIC, FX_TIME, FX_COMMIT, FX_END = 0x31465844, 0x31545844, 0x31435844, 0x444E4544  # 'DXF1' 'DXT1' 'DXC1' 'DEND'
FX_HDR = struct.Struct("<IIHHIQ6i")
FX_SEC = struct.Struct("<II")
FX_META = struct.Struct("<4B3i2f20s2s2ii3i2i2i3if5f6f3fi")
FX_META_KEYS = ("final_stage", "level", "flags", "snaps", "cem_note", "legacy_played", "legacy_n_cand", "legacy_score", "d0",
                "played", "played_grip", "played_cand", "played_fin", "n_round1", "lp_c0", "lp_cn", "lp_fin", "ms_final", "ms_write",
                "capture_us", "build_us", "budget_ms", "ko_full_ms", "lp_ms", "pick_u", "xw_worst", "anchor", "tau", "pick1_w", "lp_play",
                "points_w", "dm_w", "dm_cont", "carry_w", "goal_scale", "chase_w", "rg_w", "rg_end_w", "rg_min", "rg_sum")
FX_REP = struct.Struct("<20s2sBB")
FX_CALL = struct.Struct("<BBHHHIi")
FX_CAND = struct.Struct("<20s2sBBBBhHIff")
FX_FIN = struct.Struct("<i6sBBf6f6ffifff")
FX_PLAN = struct.Struct("<HBBB4bBBB5f")      # + "2f" (rg1, rg_end) when the PLAN section's row size is 40
FX_ROWS = struct.Struct("<HHI")              # a row section starts: row size, 0, rows (step by the row size given, not by ours)
FX_KO = struct.Struct("<HH30i9fB3x48s40s")
FX_KO_KEYS = ("hdr_size", "move_size", "choice", "would", "n_pool", "n_rep", "n_r1", "r1_ran", "n_surv", "keep_eff", "n_search", "n_mut", "n_rand", "n_new",
              "sims", "noans", "dispatchers", "disp_fail", "judged", "rejected", "passed", "pick_rank", "n_rank", "veto_n", "veto_first",
              "fresh_n", "ms_start", "ms_end", "ms_r1", "ms_r2", "ms_safe", "ms_stage", "lcb_k", "w_mean", "w_se", "w_lb", "fresh_d",
              "fresh_dse", "rg_min", "rg_over", "rg_pts", "flags", "status", "decision")
FX_KO_MOVE = struct.Struct("<hBB4f")
FX_OPEN = struct.Struct("<4HB3x6i9f7i")
FX_OPEN_KEYS = ("hdr_size", "rep_size", "cand_size", "agg_size", "status", "n_cand", "n_rep", "n_top", "n_elig", "pick", "opp_games", "lambda", "cover", "u", "near_pts", "margin",
                "lcb_k", "tau", "best_rank", "covmax", "ms1", "ms2", "n1", "ran1", "n_states", "states_done", "sims2")
FX_OPEN_REP = struct.Struct("<20sf")
FX_OPEN_CAND = struct.Struct("<20sBB2x3f")
FX_OPEN_AGG = struct.Struct("<2i8fBB2x")
FX_TIME_REC = struct.Struct("<IIQi4I")
FX_COMMIT_REC = struct.Struct("<IIQ5iB3xf")
FX_STAGE = {0: "no search", 1: "round one's pick (no finalist stage)", 3: "rescue pick (nothing was safe)", 4: "final pick over the finalists",
            5: "knockout override", 10: "opening search", 11: "book draw (opening search fell back)"}
FX_PARTS = ("stage", "anchor", "held_out", "adversarial", "counter", "one_turn", "launch_bonus", "other")  # they add up to "final"
FX_POINTS = ("start", "held-out", "adversarial", "counter", "one-turn term", "final")            # the six points score[] / safe[] / fmin[] are taken at
FX_REP_SETS = ((1, "round one"), (2, "held-out"), (4, "adversarial"), (8, "counter"), (16, "knockout"), (32, "keeps his joints"), (64, "all-hold"))
FX_COMMIT_WHY = {0: "other", 1: "search done", 2: "opponent committed", 3: "deadline", 4: "over budget"}


def _fx_j(b):
    return "".join(str(int(x)) for x in b)


def full_path(legacy_path):
    d, base = os.path.split(legacy_path)
    return os.path.join(d, "decfull_" + base[4:-4] + ".dxf") if base.startswith("dec_") and base.endswith(".bin") else None


def _fx_decision(data, o, size):
    h = FX_HDR.unpack_from(data, o)
    d = dict(zip(("magic", "size", "version", "hdr_size", "n_sections", "ts_ns", "tag", "handle", "player", "slot", "turn", "pid"), h))
    d.update(kind="decision", meta=None, reps=[], calls=[], cands=[], finalists=[], ko=None, opening=None, commit=None, time=None)
    p, end, plans = o + d["hdr_size"], o + size - 4, []
    while p + FX_SEC.size <= end:
        sid, n = FX_SEC.unpack_from(data, p)
        q = p + FX_SEC.size
        p = q + n
        if p > end:
            break
        name = data[q - 8:q - 4].decode("latin-1")
        if name == "META":
            m = dict(zip(FX_META_KEYS, FX_META.unpack_from(data, q)))
            m["played"], m["played_grip"] = _fx_j(m["played"]), _fx_j(m["played_grip"])
            m["stage_name"] = FX_STAGE.get(m["final_stage"], "?")
            d["meta"] = m
        elif name in ("REPS", "CALL", "CAND", "FINL", "PLAN"):
            row, _, cnt = FX_ROWS.unpack_from(data, q)
            at = [q + FX_ROWS.size + i * row for i in range(cnt)]
            if name == "REPS":
                for b in at:
                    j, g, src, sets = FX_REP.unpack_from(data, b)
                    d["reps"].append(dict(joints=_fx_j(j), grip=_fx_j(g), src=src, src_name=XP_SRC.get(src, "unknown (DECAP_EXPLAIN off)"), sets=sets,
                                          set_names=[nm for bit, nm in FX_REP_SETS if sets & bit]))
            elif name == "CALL":
                for b in at:
                    ph, fl, nm, nt, _, ans, ms = FX_CALL.unpack_from(data, b)
                    d["calls"].append(dict(phase=ph, flags=fl, n_moves=nm, n_replies=nt, answered=ans, ms=ms))
            elif name == "CAND":
                for b in at:
                    j, g, tag, origin, safe, npl, fin, _, first, mean_v, min_v = FX_CAND.unpack_from(data, b)
                    d["cands"].append(dict(joints=_fx_j(j), grip=_fx_j(g), tag=tag, origin=origin, safe=safe, fin=fin, mean_v=mean_v, min_v=min_v,
                                           n_plans=npl, plan_first=first, plans=[]))
            elif name == "FINL":
                for b in at:
                    v = FX_FIN.unpack_from(data, b)
                    d["finalists"].append(dict(cand=v[0], safe=[int(x) for x in v[1]], flags=v[2], logp=v[4], score=list(v[5:11]), fmin=list(v[11:17]),
                                               fmean=v[17], fcnt=v[18], rg_mean=v[19], rg_max=v[20], lp_j=v[21], played=bool(v[2] & 1),
                                               planned_launch=bool(v[2] & 2), got_launch_bonus=bool(v[2] & 4), attacked=bool(v[2] & 8)))
            else:
                for b in at:
                    v = FX_PLAN.unpack_from(data, b)
                    rg = struct.unpack_from("<2f", data, b + FX_PLAN.size) if row >= FX_PLAN.size + 8 else (0.0, 0.0)
                    plans.append(dict(rep=v[0], call=v[1], flags=v[2], flags2=v[3], dm_op1=v[4], dm_me1=v[5], dm_op_c=v[6], dm_me_c=v[7], timer1=v[8],
                                      low_cm=v[9], v=v[11], value=v[12], dealt1=v[13], taken1=v[14], points=v[15], rg1=rg[0], rg_end=rg[1],
                                      simulated=bool(v[2] & 1), unsafe=bool(v[3] & 4) or not (v[2] & 1)))
        elif name == "KOST":
            k = dict(zip(FX_KO_KEYS, FX_KO.unpack_from(data, q)))
            k["status"] = k["status"].split(b"\0")[0].decode("latin-1")
            k["decision"] = k["decision"].split(b"\0")[0].decode("latin-1")
            b = q + k["hdr_size"]
            k["replies"] = list(struct.unpack_from("<%dH" % k["n_rep"], data, b))
            b += 2 * k["n_rep"]
            k["pool"] = []
            for i in range(k["n_pool"]):
                c, src, fl, s1, s2, se, risk = FX_KO_MOVE.unpack_from(data, b + i * k["move_size"])
                k["pool"].append(dict(cand=c, src=src, flags=fl, s1=s1, s2=s2, se=se, risk=risk, round2=bool(fl & 1), own_loss=bool(fl & 2),
                                      ranked=bool(fl & 4), passes_margin=bool(fl & 8), joints=None))
            b += k["n_pool"] * k["move_size"]
            new = [x for x in k["pool"] if x["cand"] < 0]
            for i, x in enumerate(new[:k["n_new"]]):
                x["joints"] = _fx_j(data[b + 20 * i:b + 20 * i + 20])
            d["ko"] = k
        elif name == "OPEN":
            op = dict(zip(FX_OPEN_KEYS, FX_OPEN.unpack_from(data, q)))
            b = q + op["hdr_size"]
            nc, nr = op["n_cand"], op["n_rep"]
            op["replies"] = [dict(joints=_fx_j(j), w=w) for j, w in (FX_OPEN_REP.unpack_from(data, b + i * op["rep_size"]) for i in range(nr))]
            b += nr * op["rep_size"]
            op["cands"] = []
            for i in range(nc):
                j, tag, safe1, t1, prior, blend = FX_OPEN_CAND.unpack_from(data, b + i * op["cand_size"])
                op["cands"].append(dict(joints=_fx_j(j), tag=tag, safe1=safe1, t1=t1, prior=prior, blend=blend))
            b += nc * op["cand_size"]
            nets = struct.unpack_from("<%df" % (nc * nr), data, b)
            b += 4 * nc * nr
            fl = data[b:b + nc * nr]
            b += nc * nr
            for i, c in enumerate(op["cands"]):  # (per reply: the opener's turn-1 net, NaN = no answer; unsafe flags)
                c["net"] = list(nets[i * nr:(i + 1) * nr])
                c["unsafe"] = [bool(x & 2) or not (x & 1) for x in fl[i * nr:(i + 1) * nr]]
            op["top"] = []
            for i in range(op["n_top"]):
                v = FX_OPEN_AGG.unpack_from(data, b + i * op["agg_size"])
                op["top"].append(dict(zip(("c", "nd", "tot", "t1p", "t2p", "se", "var", "cov", "rank", "p", "bad", "elig"), v)))
                op["top"][-1]["joints"] = op["cands"][v[0]]["joints"] if 0 <= v[0] < nc else None
            d["opening"] = op
    for c in d["cands"]:  # (each candidate's plans; written for the finalists and the move played, for all at DECAP_LOG_FULL=2)
        c["plans"] = plans[c["plan_first"]:c["plan_first"] + c["n_plans"]]
    for k, f in enumerate(d["finalists"]):
        if 0 <= f["cand"] < len(d["cands"]):
            f["joints"] = d["cands"][f["cand"]]["joints"]
    if d["ko"]:
        for x in d["ko"]["pool"]:
            if x["joints"] is None and 0 <= x["cand"] < len(d["cands"]):
                x["joints"] = d["cands"][x["cand"]]["joints"]
    return d


def full_records(path):
    """every record of a decfull_*.dxf file, in file order; a torn record is skipped (the next known magic is looked for)"""
    data = open(path, "rb").read()
    o, known = 0, (FX_MAGIC, FX_TIME, FX_COMMIT)
    while o + 8 <= len(data):
        magic, size = struct.unpack_from("<II", data, o)
        ok = magic in known and size >= 8 and o + size <= len(data)
        if ok and magic == FX_MAGIC:
            ok = size >= FX_HDR.size + 4 and struct.unpack_from("<I", data, o + size - 4)[0] == FX_END
        elif ok:  # (the two small record types: a plausible size, so that stray bytes after a torn record are not taken for one)
            ok = (32 if magic == FX_TIME else FX_COMMIT_REC.size) <= size <= 256
        if not ok:
            nxt = min([i for i in (data.find(m, o + 1) for m in (b"DXF1", b"DXT1", b"DXC1")) if i >= 0] or [len(data)])
            o = nxt
            continue
        if magic == FX_MAGIC:
            yield _fx_decision(data, o, size)
        elif magic == FX_TIME:
            v = FX_TIME_REC.unpack_from(data, o) if size >= FX_TIME_REC.size else struct.unpack_from("<IIQi3I", data, o) + (0,)
            yield dict(kind="time", ts_ns=v[2], pid=v[3], build_us=v[4], write_us=v[5], bytes=v[6], open_us=v[7])
        else:
            v = FX_COMMIT_REC.unpack_from(data, o)
            yield dict(kind="commit", ts_ns=v[2], pid=v[3], player=v[4], turn=v[5], ms=v[6], stage=v[7], why=FX_COMMIT_WHY.get(v[8], "?"), score=v[9])
        o += size


def full_decisions(path):
    """the decisions of one .dxf file; d["time"] = its timing record (tests), d["commit"] = the commit record for it, when
    present. A commit record with no decision before it is returned in the list as a dict with kind "commit" only."""
    out, last = [], None
    for r in full_records(path):
        if r["kind"] == "decision":
            out.append(r)
            last = r
        elif r["kind"] == "time" and last is not None and last["ts_ns"] == r["ts_ns"]:
            last["time"] = r
        elif r["kind"] == "commit":
            if last is not None and last["commit"] is None and last["player"] == r["player"] and last["turn"] == r["turn"] and 0 <= r["ts_ns"] - last["ts_ns"] < 5e9:
                last["commit"] = r
            else:
                out.append(r)
    return out


def full_index(dir_or_file):
    files = [dir_or_file] if os.path.isfile(dir_or_file) else sorted(glob.glob(os.path.join(dir_or_file, "decfull_*.dxf")))
    idx = {}
    for f in files:
        for d in full_decisions(f):
            if d["kind"] == "decision":
                idx[(d["ts_ns"], d["tag"], d["handle"], d["player"], d["turn"])] = d
    return idx


def full_record(r, legacy_path, _cache={}):
    """the full record written beside legacy record r (from records(legacy_path)); None when the switch was off"""
    p = full_path(legacy_path)
    if not p or not os.path.exists(p):
        return None
    key = (p, os.path.getsize(p))
    if key not in _cache:
        _cache.clear()
        _cache[key] = full_index(p)
    return _cache[key].get((r["ts_ns"], r["tag"], r["handle"], r["player"], r["turn"]))


def finalists_table(d):
    """one row per finalist, best final score first: its parts (FX_PARTS; they add up to "final"), its safety flag after
    each stage, the mean one-turn net and mean P(win) over its guessed replies, the guard's risks and the knockout's means"""
    m, rows = d["meta"], []
    xw = m["xw_worst"]
    ko = {x["cand"]: x for x in d["ko"]["pool"] if x["cand"] >= 0} if d["ko"] else {}
    nan = float("nan")
    for k, f in enumerate(d["finalists"]):
        c = d["cands"][f["cand"]]
        s = f["score"]
        stage = (1.0 - xw) * c["mean_v"] + xw * c["min_v"]
        # (between the one-turn term and the final pick B35 / B36 change a score only by the planned-launch bonus. A later
        # build may charge something else there, e.g. B37's worst-guessed-reply charge: that shows as "other", 0 in B36.)
        bonus = m["lp_play"] if f["got_launch_bonus"] else 0.0
        guess = [p for p in c["plans"] if p["simulated"] and not (d["reps"][p["rep"]]["sets"] & 8)]
        one = [p["dealt1"] - p["taken1"] for p in guess]
        r1 = [p["value"] for p in c["plans"] if p["simulated"] and 0 < p["call"] <= len(d["calls"]) and d["calls"][p["call"] - 1]["phase"] == 0]
        rows.append(dict(fin=k, cand=f["cand"], joints=c["joints"], tag=c["tag"], origin=c["origin"], played=f["played"], safe=f["safe"][5],
                         safe_after=f["safe"], stage=stage, anchor=m["anchor"] * f["logp"], held_out=s[1] - s[0], adversarial=s[2] - s[1],
                         counter=s[3] - s[2], one_turn=s[4] - s[3], launch_bonus=bonus, other=(s[5] - s[4]) - bonus, final=s[5], start=s[0], fmin=f["fmin"][5],
                         logp=f["logp"], n_plans=len(c["plans"]), one_turn_mean=sum(one) / len(one) if one else nan,
                         value_mean=sum(r1) / len(r1) if r1 else nan, attacked=f["attacked"], rg_mean=f["rg_mean"], rg_max=f["rg_max"],
                         ko_s1=ko[f["cand"]]["s1"] if f["cand"] in ko else nan, ko_s2=ko[f["cand"]]["s2"] if f["cand"] in ko else nan))
    rows.sort(key=lambda r: -(r["final"] if r["final"] == r["final"] else -1e9))
    return rows


def final_pick(d, scores=None, safe=None):
    """the finalist the final pick chooses (its row in d["finalists"]; -1: none safe): the bot's own rule
    (bot::pickCandidate: softmax of score / tau over the safe finalists, read off with one uniform draw) replayed with
    the decision's own temperature and draw (META.tau, META.pick_u). scores / safe default to the logged final ones;
    pass other scores to see what another rule would have played with the same draw. Rescue decisions made no draw:
    the first finalist was played."""
    import math
    m, fin = d["meta"], d["finalists"]
    score = [f["score"][5] for f in fin] if scores is None else list(scores)
    ok = [f["safe"][5] == 1 for f in fin] if safe is None else [bool(x) for x in safe]
    if m["flags"] & 1:
        return 0 if fin else -1
    s = [(x if x == x and abs(x) != float("inf") else -1e9) for x in score]
    top = -1
    for c in range(len(fin)):
        if ok[c] and (top < 0 or s[c] > s[top]):
            top = c
    if top < 0 or not (m["tau"] > 0.0) or m["pick_u"] < 0:
        return top
    w = [math.exp((s[c] - s[top]) / m["tau"]) if ok[c] else 0.0 for c in range(len(fin))]
    r = m["pick_u"] * sum(w)
    for c in range(len(fin)):
        if w[c] > 0.0:
            r -= w[c]
            if r < 0.0:
                return c
    return top


def decided_by(d):
    """which stage produced the move played, and for a finalist-stage decision the point (FX_POINTS) from which the final
    pick's finalist stayed the best safe one; None there = it was not the top score (the softmax draw or the knockout chose)"""
    m = d["meta"]
    out = dict(stage=m["final_stage"], name=m["stage_name"], led_from=None, normal_pick=None)
    pick = m["played_fin"]
    if d["ko"] and d["ko"]["pool"] and d["ko"]["pool"][0]["cand"] >= 0 and d["cands"]:
        pick = d["cands"][d["ko"]["pool"][0]["cand"]]["fin"]  # (the normal pick: what the finalist stage chose before the knockout)
        out["normal_pick"] = pick
    if pick is None or pick < 0 or not d["finalists"]:
        return out
    tops = []
    for s in range(6):
        best = None
        for k, f in enumerate(d["finalists"]):
            if f["safe"][s] == 1 and f["score"][s] == f["score"][s] and (best is None or f["score"][s] > d["finalists"][best]["score"][s]):
                best = k
        tops.append(best)
    out["top_after"] = tops
    for s in range(5, -1, -1):
        if tops[s] != pick:
            break
        out["led_from"] = FX_POINTS[s]
    return out


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2:])
