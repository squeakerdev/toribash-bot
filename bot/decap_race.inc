// decapAI bot, build B53: the race: a wide pool of one-turn moves cut down in rounds as guessed replies are added.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

static std::map<std::string, float> decap_race_mean;

struct decap_race_info {
	int pool = 0, nrep = 0, sims = 0, noans = 0, mutants = 0, disp_fail = 0, stages = 0;
	long ms = 0;
	float best_mean = NAN;
};

static bool decap_race_parse(const char *name, const char *def, std::vector<int> &out)
{
	const char *s = getenv(name);
	if (!s || !*s) s = def;
	out.clear();
	for (const char *p = s; *p;) {
		char *e = nullptr;
		long v = strtol(p, &e, 10);
		if (e == p) break;
		out.push_back((int)v);
		p = (*e == ',') ? e + 1 : e;
		if (*e != ',') break;
	}
	return !out.empty();
}

static void decap_race_call(int seat, const std::vector<Move> &M, const std::vector<Move> &R, int n_frames,
                            float skill_level, unsigned seed, long left, std::vector<decap_outcome> &out, decap_race_info &info)
{
	// [REDACTED: this function mainly reads the game's state; 86 lines]
}

static bool decap_race_select(int seat, const std::vector<Move> &mine, const std::vector<Move> &replies_in,
                              int n_frames, float skill_level, unsigned seed, const struct timespec &t0, long end_ms,
                              std::vector<Move> &out, std::vector<float> &out_mean, decap_race_info &info)
{
	std::vector<int> sizes, reps, mut1, mut2;
	decap_race_parse("DECAP_RACE_SIZES", "512,128,32,8", sizes);
	decap_race_parse("DECAP_RACE_REPS", "2,6,12,20", reps);
	decap_race_parse("DECAP_RACE_MUT1", "8,8", mut1);
	decap_race_parse("DECAP_RACE_MUT2", "0,0", mut2);
	if (sizes.size() != reps.size() || sizes.empty() || mine.empty() || replies_in.empty()) return false;
	const int op = 1 - seat;
	auto key = [](const Move &r) { return std::string((const char *)r.states, N_JOINTS); };
	auto mutate = [&](Move r, int who) {
		int k = 1 + rand() % 3, done = 0;
		bool used[N_JOINTS] = {};
		for (int tries = 0; done < k && tries < 64; tries++) {
			int j = rand() % N_JOINTS;
			// [game access]
			used[j] = true;
			int st = 1 + rand() % 3;
			if (st >= r.states[j]) st++;
			r.states[j] = (uint8_t)st;
			done++;
		}
		return r;
	};

	std::vector<Move> P;
	std::set<std::string> seen;
	for (const Move &r : mine) if (seen.insert(key(r)).second) P.push_back(r);
	const size_t n_base = P.size();
	for (int t = 0; (int)P.size() < sizes[0] && t < sizes[0] * 40; t++) {
		Move m = mutate(P[t % n_base], seat);
		if (seen.insert(key(m)).second) P.push_back(m);
	}

	const int nrep_max = std::min(64, reps.back());
	std::vector<Move> R;
	std::set<std::string> rs;
	for (const Move &r : replies_in) if ((int)R.size() < nrep_max && rs.insert(key(r)).second) R.push_back(r);
	const size_t n_guess = R.size();
	for (int t = 0; (int)R.size() < nrep_max && t < 4000; t++) {
		Move m = mutate(R[t % n_guess], op);
		if (rs.insert(key(m)).second) R.push_back(m);
	}
	info.pool = (int)P.size();
	info.nrep = (int)R.size();

	struct S { double sum = 0.0; int n = 0; int loss = 0; int have = 0; };
	std::vector<S> sc(P.size());
	auto score_to = [&](const std::vector<size_t> &idx, int r_to, unsigned sd) {
		std::map<int, std::vector<size_t>> by_from;
		for (size_t i : idx) if (sc[i].have < r_to) by_from[sc[i].have].push_back(i);
		for (auto &kv : by_from) {
			const int r0 = kv.first;
			std::vector<Move> M, RR(R.begin() + r0, R.begin() + std::min((int)R.size(), r_to));
			for (size_t i : kv.second) M.push_back(P[i]);
			std::vector<decap_outcome> o;
			const long left = end_ms - decap_ms_since(t0);
			if (left <= 0) return false;
			decap_race_call(seat, M, RR, n_frames, skill_level, sd ^ (unsigned)(r0 * 0x9e37u), left, o, info);
			for (size_t q = 0; q < kv.second.size(); q++) {
				S &s = sc[kv.second[q]];
				for (size_t r = 0; r < RR.size(); r++) {
					const decap_outcome &x = o[q * RR.size() + r];
					if (x.ok != 1 || !std::isfinite(x.dealt1) || !std::isfinite(x.taken1)) { s.loss = 1; continue; }
					s.sum += x.dealt1 - x.taken1;
					s.n++;
					if (x.lost1) s.loss = 1;
				}
				s.have = std::min((int)R.size(), r_to);
			}
		}
		return decap_ms_since(t0) < end_ms;
	};
	auto mean = [&](size_t i) { return sc[i].n > 0 ? sc[i].sum / sc[i].n : -1e30; };
	auto ranked = [&](std::vector<size_t> v) {
		std::stable_sort(v.begin(), v.end(), [&](size_t a, size_t b) {
			if (sc[a].loss != sc[b].loss) return sc[a].loss < sc[b].loss;
			return mean(a) > mean(b);
		});
		return v;
	};
	std::vector<size_t> C;
	for (size_t i = 0; i < P.size() && (int)i < sizes[0]; i++) C.push_back(i);
	for (size_t st = 0; st < sizes.size(); st++) {
		const int r_to = std::min((int)R.size(), reps[st]);
		if (!score_to(C, r_to, seed ^ (0x52414345u + (unsigned)st))) return false;
		std::vector<size_t> ord = ranked(C);
		const std::vector<int> *mu = st == 1 ? &mut1 : (st == 2 ? &mut2 : nullptr);
		if (mu && mu->size() == 2 && (*mu)[0] > 0 && (*mu)[1] > 0) {
			std::vector<size_t> nw;
			for (int p = 0; p < (*mu)[0] && p < (int)ord.size(); p++) {
				for (int got = 0, t = 0; got < (*mu)[1] && t < 200; t++) {
					Move m = mutate(P[ord[p]], seat);
					if (!seen.insert(key(m)).second) continue;
					P.push_back(m);
					sc.push_back(S());
					nw.push_back(P.size() - 1);
					got++;
				}
			}
			info.mutants += (int)nw.size();
			if (!nw.empty()) {
				if (!score_to(nw, r_to, seed ^ (0x4d555400u + (unsigned)st))) return false;
				ord.insert(ord.end(), nw.begin(), nw.end());
				ord = ranked(ord);
			}
		}
		info.stages++;
		if (st + 1 < sizes.size()) ord.resize(std::min(ord.size(), (size_t)std::max(1, sizes[st + 1])));
		C = ord;
	}
	out.clear();
	out_mean.clear();
	for (size_t i : C) {
		out.push_back(P[i]);
		out_mean.push_back((float)mean(i));
	}
	info.best_mean = C.empty() ? NAN : (float)mean(C[0]);
	return !out.empty();
}

static float decap_race_one(const Move &r, float fallback)
{
	std::map<std::string, float>::const_iterator it = decap_race_mean.find(std::string((const char *)r.states, N_JOINTS));
	return it != decap_race_mean.end() && std::isfinite(it->second) ? it->second : fallback;
}

static bool decap_race_hook(int seat, std::vector<Move> &mine, const std::vector<Move> &theirs,
                            const Position &their_pos, int n_frames, float skill_level, const struct timespec &t0, float d0)
{
	// [REDACTED: this function mainly reads the game's state; 56 lines]
}

static char decap_rr_scratch[128];
struct decap_rr_snap_t {
	bool ok = false;
	int32_t rs[32];
	std::vector<Move> mine;
	decap_diag_t diag;
	decap_full_t full;
	decap_xp_t xp;
	decap_lf_t lf;
	int t1_mode, t1_every, t1_bad, t1_emit, t1_have;
	std::unordered_map<std::string, decap_outcome> t1_cache;
	decltype(decap_t1_stats) t1_stats;
	decltype(decap_decision_scripts) scripts;
	decltype(decap_rg_seen) rg_seen;
};
static decap_rr_snap_t decap_rr_snap;
static bool decap_rr_rand_io(int32_t *buf, bool save)
{
	// [REDACTED: this function mainly reads the game's state; 9 lines]
}

static bool decap_rr_begin(const std::vector<Move> &mine)
{
	// [REDACTED: this function mainly reads the game's state; 16 lines]
}

static void decap_rr_restore(std::vector<Move> &mine, const decap_pick &pick, const struct timespec &t0, float d0)
{
	// [REDACTED: this function mainly reads the game's state; 38 lines]
}
