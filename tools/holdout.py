"""Unbiased numbers for decapAI tests (written 14 Nov 2026).

Problem: a number measured on the same positions that were used to pick or tune an idea comes out too high, because the
picking favours lucky results. Fix: positions used for picking never produce a quoted number.

Games are split ONCE, by a hash of the game's start time, into two sides:
  explore (60%)  free to look at, tune on, split after the fact. Numbers from it only decide "build it or not".
  final   (40%)  one run per built binary, rule file written first. Its number is the one that gets quoted.
The hash cannot be nudged, needs no file, and gives the same answer on Windows and in WSL.

A final run consumes its positions for that idea: a later build of the same idea must use other final positions.

Use in a driver:
    import holdout
    states = holdout.keep(all_states, "explore")            # first looks
    holdout.first_look("planner", "pop80 gens5", n, est, lo, hi, "close2 emulation")
    holdout.register_final("planner", md5, rule_path, states)   # before the first result row; raises if not allowed
    holdout.record_result("planner", md5, n, est, lo, hi, "not shown")
Command line: python holdout.py stats <games.json> | ledger | looks | side <game time> | selftest
"""
import hashlib, json, os, sys, time

SALT = "decapai-holdout-v1"
FINAL_PCT = 40
DIR = "<notes>/holdout"
LEDGER = DIR + "/LEDGER.jsonl"
LOOKS = DIR + "/FIRST_LOOKS.jsonl"


def _gt(s, gt_key="gt"):
    v = s[gt_key] if isinstance(s, dict) else (s[0] if isinstance(s, (list, tuple)) else s)
    return int(float(v))


def _pos(s, gt_key="gt", k_key="k"):
    if isinstance(s, dict):
        return [_gt(s, gt_key), int(s[k_key])]
    return [int(float(s[0])), int(s[1])]


def side(gt):
    h = hashlib.sha256(("%s:%d" % (SALT, int(float(gt)))).encode()).digest()
    return "final" if (h[0] * 256 + h[1]) % 100 < FINAL_PCT else "explore"


def keep(states, purpose, gt_key="gt"):
    """Return only the states on the given side ('explore' or 'final')."""
    assert purpose in ("explore", "final")
    return [s for s in states if side(_gt(s, gt_key)) == purpose]


def check(states, purpose, gt_key="gt"):
    """Stop the program if any state is on the wrong side."""
    bad = [s for s in states if side(_gt(s, gt_key)) != purpose]
    if bad:
        raise SystemExit("holdout: %d of %d states are on the wrong side for a '%s' list" % (len(bad), len(states), purpose))
    return states


def _append(path, row):
    os.makedirs(DIR, exist_ok=True)
    row = dict(row, when=time.strftime("%Y-%m-%d %H:%M:%S"))
    with open(path, "a", encoding="utf-8") as f:
        f.write(json.dumps(row) + "\n")
    return row


def _read(path):
    if not os.path.exists(path):
        return []
    return [json.loads(l) for l in open(path, encoding="utf-8") if l.strip()]


def first_look(idea, variant, n, est, lo, hi, source, note=""):
    """Log every idea and variant tried, so 'best of how many' is on record."""
    return _append(LOOKS, dict(idea=idea, variant=variant, n=n, est=est, lo=lo, hi=hi, source=source, note=note))


def used(idea):
    """Final positions already consumed by this idea (set of (gt, k))."""
    out = set()
    for r in _read(LEDGER):
        if r.get("idea") == idea and r.get("kind") in ("final", "retired"):
            out.update((p[0], p[1]) for p in r.get("positions", []))
    return out


def retire(idea, states, reason, gt_key="gt", k_key="k"):
    """Mark positions as already seen by an idea (for runs made before this file existed)."""
    pos = sorted({tuple(_pos(s, gt_key, k_key)) for s in states})
    return _append(LEDGER, dict(kind="retired", idea=idea, reason=reason, n=len(pos), positions=[list(p) for p in pos]))


def register_final(idea, binary_md5, rule_path, states, gt_key="gt", k_key="k"):
    """Call before the first result row of a final run. Raises SystemExit when the run is not allowed."""
    if not os.path.exists(rule_path):
        raise SystemExit("holdout: rule file missing: %s (write the pass rule first)" % rule_path)
    check(states, "final", gt_key)
    pos = sorted({tuple(_pos(s, gt_key, k_key)) for s in states})
    led = _read(LEDGER)
    if any(r.get("kind") == "final" and r.get("idea") == idea and r.get("binary_md5") == binary_md5 for r in led) \
            and not os.environ.get("HOLDOUT_RERUN"):
        raise SystemExit("holdout: build %s of '%s' already had its final run (one run per build)" % (binary_md5[:12], idea))
    seen = used(idea)
    clash = [p for p in pos if p in seen]
    if clash:
        raise SystemExit("holdout: %d of %d positions already judged an earlier build of '%s'; pick other final positions"
                         % (len(clash), len(pos), idea))
    rule_md5 = hashlib.md5(open(rule_path, "rb").read()).hexdigest()
    return _append(LEDGER, dict(kind="final", idea=idea, binary_md5=binary_md5, rule=rule_path, rule_md5=rule_md5,
                                n=len(pos), positions=[list(p) for p in pos],
                                rerun=os.environ.get("HOLDOUT_RERUN", "")))


def record_result(idea, binary_md5, n, est, lo, hi, verdict, measure=""):
    """Write the final number. This row is the only number to quote as a gain."""
    return _append(LEDGER, dict(kind="result", idea=idea, binary_md5=binary_md5, n=n, est=est, lo=lo, hi=hi,
                                verdict=verdict, measure=measure))


def need_n(sd_per_state, half_width):
    """States needed so the 95% interval is +-half_width, given the spread of paired differences per state."""
    return int((1.96 * sd_per_state / half_width) ** 2) + 1


def _stats(path):
    games = json.load(open(path))
    out = {"explore": [0, 0, 0], "final": [0, 0, 0]}
    for g in games:
        s = side(g["t"])
        out[s][0] += 1
        for t in g.get("turns", []):
            out[s][1] += 1
            if t.get("d0") is not None and t["d0"] < 2.5:
                out[s][2] += 1
    for s in ("explore", "final"):
        print("%-8s games %4d  turns %5d  close turns (< 2.5 m) %5d" % (s, *out[s]))
    for idea in sorted({r["idea"] for r in _read(LEDGER) if "idea" in r}):
        u = used(idea)
        print("idea %-14s final positions already consumed: %d" % (idea, len(u)))


def _selftest():
    assert side(1700000000) == side("1700000000.0")
    n = sum(side(1790000000 + i * 977) == "final" for i in range(20000))
    assert 0.38 < n / 20000 < 0.42, n
    st = [{"gt": 1790000000 + i * 977, "k": 3} for i in range(200)]
    assert len(keep(st, "final")) + len(keep(st, "explore")) == 200
    try:
        check(st, "final")
        raise AssertionError("mixed list passed")
    except SystemExit:
        pass
    assert need_n(7500, 750) == 385
    print("selftest ok: final share %.3f" % (n / 20000))


if __name__ == "__main__":
    a = sys.argv[1:]
    if a[:1] == ["stats"]:
        _stats(a[1])
    elif a[:1] == ["ledger"]:
        for r in _read(LEDGER):
            r = dict(r)
            r.pop("positions", None)
            print(json.dumps(r))
    elif a[:1] == ["looks"]:
        for r in _read(LOOKS):
            print(json.dumps(r))
    elif a[:1] == ["side"]:
        print(side(a[1]))
    elif a[:1] == ["selftest"]:
        _selftest()
    else:
        print(__doc__)
