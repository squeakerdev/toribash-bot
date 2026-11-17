// decapAI bot, build B53: the opening search (turn 0): book openers against the opponent's likely first moves, then a second-turn tournament.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

#include <semaphore.h>
#include <stdarg.h>
#include <functional>

enum { DOP_FMAX = 96, DOP_GMAX = 12, DOP_SMAX = 400, DOP_CMAX = 256, DOP_RMAX = 12, DOP_E1MAX = 6000 };

struct dop_player {
	int n = 0;
	long last = 0;
	std::map<std::string, double> first;
	std::map<std::string, std::map<std::string, double>> second;
};
struct dop_table {
	bool tried = false, ok = false;
	long build = 0;
	int games = 0;
	std::map<std::string, double> pool_first;
	std::map<std::string, std::map<std::string, double>> pool_second;
	// [game access]
	std::map<std::string, std::pair<int, double>> prior;
	std::string err;
};
static dop_table dop_t;
static bool dop_book_tried = false;
static std::vector<Move> dop_book;
static std::vector<int> dop_book_copies;
static std::map<std::string, std::set<std::string>> dop_ban;

static bool dop_move_ok(const std::string &m)
{
	if (m.size() != 20) return false;
	for (char c : m) if (c < '1' || c > '4') return false;
	return true;
}
static std::string dop_str(const uint8_t *j)
{
	std::string s(20, '0');
	for (int i = 0; i < 20; i++) s[i] = (char)('0' + j[i]);
	return s;
}
static void dop_set(Move &r, const std::string &m)
{
	for (int i = 0; i < 20; i++) r.states[i] = (uint8_t)(m[i] - '0');
}

static std::string dop_norm(const char *name)
{
	// [REDACTED: this function mainly reads the game's state; 13 lines]
}

static void dop_load_table()
{
	// [REDACTED: this function mainly reads the game's state; 46 lines]
}

static void dop_load_book()
{
	// [REDACTED: this function mainly reads the game's state; 26 lines]
}

static void decap_open_load()
{
	// [REDACTED: this function mainly reads the game's state; 13 lines]
}

static std::vector<std::pair<std::string, double>> dop_mixture(const std::string &name, double k, int *n_out, double *lam_out)
{
	// [REDACTED: this function mainly reads the game's state; 24 lines]
}

static std::vector<std::string> dop_logged_second(const std::string &name, const std::string &h, int k)
{
	// [REDACTED: this function mainly reads the game's state; 19 lines]
}

static void dop_releases(const Move &o, int mode, std::vector<Move> &out)
{
	// [REDACTED: this function mainly reads the game's state; 23 lines]
}

// [game access]
static int dop_fast_copies = 0;

static bool dop_unsafe(const decap_outcome &o, int limit, float min_low)
{
	return !o.ok || o.lost1 || (!o.over1 && (o.timer1 >= limit || o.low1 < min_low || o.near1));
}

static bool dop_gone(pid_t pid)
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}
static void dop_nap_us(long us)
{
	struct timespec ts = { 0, us * 1000 };
	nanosleep(&ts, nullptr);
}

// [game access: 2 lines]
{
	// [REDACTED: this function mainly reads the game's state; 40 lines]
}

struct dop_sim { int32_t done; float net; uint8_t unsafe, lost, pad[2]; };
struct dop_state {
	volatile int32_t phase;
	int32_t c, r, nF, nG, best, unsafe_all, sims, ms, k2, over0;
	decap_outcome o1;
	float t2, se2;
	uint8_t F[DOP_FMAX][20], Fsrc[DOP_FMAX], G[DOP_GMAX][20], Gsrc[DOP_GMAX];
	dop_sim sim[DOP_FMAX * DOP_GMAX];
};
struct dop_res1 { volatile int32_t done; decap_outcome o; };
// [game access]
struct dop_buf {
	char s[16384];
	size_t n = 0;
	void add(const char *fmt, ...) __attribute__((format(printf, 2, 3)))
	{
		if (n >= sizeof(s) - 2) return;
		va_list ap;
		va_start(ap, fmt);
		int k = vsnprintf(s + n, sizeof(s) - n, fmt, ap);
		va_end(ap);
		if (k > 0) n = std::min(sizeof(s) - 2, n + (size_t)k);
	}
};

static void dop_log(const char *line)
{
	if (!decap_env_int("DECAP_OPEN_LOG", 1)) return;
	fputs(line, stderr);
	fflush(stderr);
	const char *dir = getenv("DECAP_DECISION_LOG");
	if (dir && *dir) {
		char path[600];
		// [game access]
		int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd >= 0) {
			ssize_t w = write(fd, line, strlen(line));
			(void)w;
			close(fd);
		}
	}
}

static Move decap_open_choose(int me, const Position &req, float dl, int frames0)
{
	struct timespec t0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	const int op = 1 - me;
	// [game access]
	decap_publish(fallback, 0.0f, 1);
	decap_open_load();

	long budget = decap_env_int("DECAP_LOOKAHEAD_MS", 3000);
	// [game access]
	const long end_ms = budget * std::min(950, std::max(100, decap_env_int("DECAP_OPEN_END_PERMILLE", 800))) / 1000;
	const double kshrink = std::max(0, decap_env_int("DECAP_OPEN_K_MILLI", 8000)) / 1000.0;
	const int nrep_max = std::min((int)DOP_RMAX, std::max(1, decap_env_int("DECAP_OPEN_REPLIES", 6)));
	const double mass = decap_env_int("DECAP_OPEN_MASS_MILLI", 900) / 1000.0;
	const int top_m = std::min(64, std::max(1, decap_env_int("DECAP_OPEN_TOP", 6)));
	const double prior_w = decap_env_int("DECAP_OPEN_PRIOR_MILLI", 1000) / 1000.0;
	const double prior_k = std::max(0, decap_env_int("DECAP_OPEN_PRIOR_K", 10));
	const int fmax = std::min((int)DOP_FMAX, std::max(1, decap_env_int("DECAP_OPEN_FMAX", 48)));
	const int fdraws = std::min(64, std::max(0, decap_env_int("DECAP_OPEN_FDRAWS", 12)));
	const int fknn = std::min(64, std::max(0, decap_env_int("DECAP_OPEN_FKNN", 8)));
	const int rel_mode = decap_env_int("DECAP_OPEN_REL", 1);
	const int ng_max = std::min((int)DOP_GMAX, std::max(1, decap_env_int("DECAP_OPEN_G", 6)));
	const int g1 = std::min(ng_max, std::max(1, decap_env_int("DECAP_OPEN_G1", 2)));
	const int k2 = std::min(fmax, std::max(1, decap_env_int("DECAP_OPEN_K2", 6)));
	const int glog = std::min(ng_max, std::max(0, decap_env_int("DECAP_OPEN_GLOG", 2)));
	const float g_temp = std::max(0, decap_env_int("DECAP_OPEN_GT_MILLI", 600)) / 1000.0f;
	const double lcb_k = std::max(0, decap_env_int("DECAP_OPEN_LCB_MILLI", 1000)) / 1000.0;
	const double margin = std::max(0, decap_env_int("DECAP_OPEN_MARGIN_MILLI", 1000)) / 1000.0;
	const double tau_min = std::max(1, decap_env_int("DECAP_OPEN_TAU_MIN", 200));

	const double near_pts = std::max(0, decap_env_int("DECAP_OPEN_NEAR_PTS", 0));
	const int states_at_once = std::min(64, std::max(1, decap_env_int("DECAP_OPEN_STATES_AT_ONCE", 6)));
	const int jpar = std::min(16, std::max(1, decap_env_int("DECAP_OPEN_JPAR", 4)));
	const int limit = decap_env_int("DECAP_TIMER_LIMIT", 10);
	const float min_low = decap_env_int("DECAP_MIN_LOW_MILLI", 150) / 1000.0f;
	int par = std::min(64, std::max(1, decap_env_int("DECAP_OPEN_PAR", 20)));
	{
		cpu_set_t cs;
		CPU_ZERO(&cs);
		if (sched_getaffinity(0, sizeof(cs), &cs) == 0 && CPU_COUNT(&cs) > 0) par = std::min(par, (int)CPU_COUNT(&cs));
	}
	const unsigned alarm_s = (unsigned)(budget / 1000 + 3);
	dop_fast_copies = decap_env_int("DECAP_OPEN_FAST_COPIES", 0);

	std::string opp_override;
	bool have_override = false;
	std::vector<std::pair<std::string, std::string>> eval1, eval2;
	if (const char *ctl = getenv("DECAP_OPEN_CTL")) {
		if (FILE *f = fopen(ctl, "r")) {
			char word[96];
			std::string mode;
			while (fscanf(f, "%95s", word) == 1) {
				std::string w(word);
				if (w == "opp" || w == "eval1" || w == "eval2") { mode = w; continue; }
				// [game access]
				else if (mode == "eval1" || mode == "eval2") {
					size_t c = w.find(':');
					if (c == 20 && dop_move_ok(w.substr(0, 20)) && dop_move_ok(w.substr(21))) (mode == "eval1" ? eval1 : eval2).push_back(std::make_pair(w.substr(0, 20), w.substr(21)));
				}
			}
			fclose(f);
		}
	}
	const bool eval_mode = !eval1.empty() || !eval2.empty();

	std::string opp;
	if (have_override) opp = opp_override;
	// [game access: 4 lines]
	}
	static dop_buf lb;
	lb.n = 0;
	lb.s[0] = 0;

	std::vector<Move> rec_c;
	std::vector<uint8_t> rec_tag, rec_safe;
	std::vector<float> rec_t1, rec_tot, rec_rank;
	auto finish = [&](const Move &played, const char *why, int note, float score) {
		lb.add("; %s; %ld ms\n", why, decap_ms_since(t0));
		dop_log(lb.s);
		if (!decap_env_int("DECAP_OPEN_REC", 1)) return;
		decap_xp_reset();
		decap_last_dec.req = req;
		decap_last_dec.cands.clear();
		int played_idx = -1;
		for (size_t c = 0; c < rec_c.size() && (int)decap_last_dec.cands.size() < DECAP_DIAG_MAX - 1; c++) {
			decap_dec_cand d;
			memset(&d, 0, sizeof(d));
			memcpy(d.joints, rec_c[c].states, N_JOINTS);
			d.grip[0] = rec_c[c].grip[0];
			d.grip[1] = rec_c[c].grip[1];
			d.safe = rec_safe[c];
			d.tag = rec_tag[c];
			d.min_v = rec_t1[c];
			d.mean_v = rec_tot[c];
			d.fscore = rec_rank[c];
			if (played_idx < 0 && memcmp(d.joints, played.states, N_JOINTS) == 0) played_idx = (int)decap_last_dec.cands.size();
			decap_last_dec.cands.push_back(d);
		}
		if (played_idx < 0) {
			decap_dec_cand d;
			memset(&d, 0, sizeof(d));
			memcpy(d.joints, played.states, N_JOINTS);
			d.grip[0] = played.grip[0];
			d.grip[1] = played.grip[1];
			d.tag = 14;
			d.min_v = d.mean_v = d.fscore = NAN;
			played_idx = (int)decap_last_dec.cands.size();
			decap_last_dec.cands.push_back(d);
		}
		decap_last_dec.played_idx = played_idx;
		decap_last_dec.played = played;
		decap_last_dec.cem_note = note;
		decap_last_dec.ready = 0;
		float uu[2];
		decap_last_dec.d0 = decap_chest_dist(me, op, uu);
		decap_last_dec.score = score;
		decap_last_dec.valid = true;
	};
	// [game access]
	if (!dop_t.ok || dop_book.empty()) {
		// [game access: 2 lines]
		return fallback;
	}

	int opp_n = 0;
	double lam = 0.0;
	std::vector<std::pair<std::string, double>> mix = dop_mixture(opp, kshrink, &opp_n, &lam);
	std::vector<std::string> Rm;
	std::vector<double> Rw;
	double cum = 0.0;
	for (const auto &kv : mix) {
		if ((int)Rm.size() >= nrep_max || cum >= mass) break;
		Rm.push_back(kv.first);
		Rw.push_back(kv.second);
		cum += kv.second;
	}

	std::vector<Move> C = dop_book;
	std::vector<uint8_t> Ctag(C.size(), 14);
	int n_extra_listed = 0, n_extra_excluded = 0;
	if (const char *ex = getenv("DECAP_OPEN_EXTRA")) {
		const int min_games = std::max(1, decap_env_int("DECAP_OPEN_EXTRA_MIN_GAMES", 5));
		// [game access: 2 lines]
		std::string s(ex);
		size_t at = 0;
		while (at < s.size()) {
			size_t e = s.find(',', at);
			if (e == std::string::npos) e = s.size();
			std::string m = s.substr(at, e - at);
			at = e + 1;
			if (!dop_move_ok(m)) continue;
			n_extra_listed++;
			if (!known) continue;
			// [game access]
			auto bi = dop_ban.find(m);
			// [game access: 2 lines]
			Move r = C[0];
			dop_set(r, m);
			bool dup = false;
			for (const Move &q : C) dup = dup || memcmp(q.states, r.states, 20) == 0;
			if (!dup && C.size() < DOP_CMAX) { C.push_back(r); Ctag.push_back(15); }
		}
	}
	if (C.size() > DOP_CMAX) C.resize(DOP_CMAX);
	const int nC = (int)C.size();
	if (eval_mode) {
		Rm.clear();
		Rw.clear();
	}
	// [game access: 2 lines]
	Move hold = fallback;
	for (int j = 0; j < 20; j++) hold.states[j] = 3;

	std::vector<std::pair<int, int>> jobs1;
	std::vector<Move> Rr;
	std::vector<Move> E1c;
	if (!eval_mode) {
		for (const std::string &m : Rm) { Move r = fallback; dop_set(r, m); Rr.push_back(r); }
		for (int c = 0; c < nC; c++) for (int r = 0; r < (int)Rr.size(); r++) jobs1.push_back(std::make_pair(c, r));
	}
	else {
		for (const auto &pr : eval1) {
			if (jobs1.size() >= DOP_E1MAX) break;
			Move a = fallback, b = fallback;
			dop_set(a, pr.first);
			dop_set(b, pr.second);
			E1c.push_back(a);
			Rr.push_back(b);
			jobs1.push_back(std::make_pair((int)E1c.size() - 1, (int)Rr.size() - 1));
		}
	}
	const int n1 = (int)jobs1.size();
	const int smax = eval_mode ? std::min((int)DOP_SMAX, (int)eval2.size()) : std::min((int)DOP_SMAX, top_m * (int)Rr.size());
	size_t sz = sizeof(dop_hdr) + (size_t)std::max(1, n1) * sizeof(dop_res1) + (size_t)std::max(1, smax) * sizeof(dop_state);
	// [game access: 3 lines]
		return fallback;
	}
	memset(shm, 0, sz);
	dop_hdr *hdr = (dop_hdr *)shm;
	dop_res1 *res1 = (dop_res1 *)(shm + sizeof(dop_hdr));
	dop_state *st = (dop_state *)(shm + sizeof(dop_hdr) + (size_t)std::max(1, n1) * sizeof(dop_res1));
	// [game access]
	auto cleanup = [&]() {
		// [game access: 3 lines]
	};

	const std::vector<Move> &C1 = eval_mode ? E1c : C;
	long ms1 = decap_ms_since(t0);
	int ran1 = dop_run(n1, [&](int k) {
		decap_plan_turn_only = 1;
		res1[k].o = decap_play_plan(me, C1[jobs1[k].first], Rr[jobs1[k].second], frames0, dl);
		__sync_synchronize();
		res1[k].done = 1;
	}, par, &hdr->tok, t0, end_ms, alarm_s);
	ms1 = decap_ms_since(t0) - ms1;
	if (eval_mode) {
		for (int k = 0; k < n1; k++) {
			const decap_outcome &o = res1[k].o;
			char b[512];
			// [game access: 3 lines]
			dop_log(b);
		}
	}

	std::vector<double> T1(nC, NAN), B(nC, NAN), prior2(nC, 0.0);
	std::vector<char> safe1(nC, 0);
	int n_unsafe1 = 0, n_noans1 = 0;
	if (!eval_mode) {
		const int nR = (int)Rr.size();
		for (int c = 0; c < nC; c++) {
			double s = 0.0, sw = 0.0;
			bool ok = true;
			for (int r = 0; r < nR; r++) {
				const dop_res1 &x = res1[c * nR + r];
				if (!x.done) { ok = false; n_noans1++; continue; }
				if (dop_unsafe(x.o, limit, min_low)) ok = false;
				s += Rw[r] * (x.o.dealt1 - x.o.taken1);
				sw += Rw[r];
			}
			if (sw > 0) T1[c] = s / sw;
			safe1[c] = ok && sw > 0;
			n_unsafe1 += !safe1[c];
			auto pi = dop_t.prior.find(dop_str(C[c].states));
			if (pi != dop_t.prior.end()) prior2[c] = pi->second.first / (pi->second.first + prior_k) * pi->second.second;
			B[c] = std::isfinite(T1[c]) ? T1[c] + prior_w * prior2[c] : NAN;
		}
	}
	// [game access]
	for (size_t r = 0; r < Rm.size(); r++) lb.add(" %s %.3f", Rm[r].c_str(), Rw[r]);
	// [game access]
	std::vector<int> top;
	for (int c = 0; c < nC; c++) if (safe1[c]) top.push_back(c);
	std::sort(top.begin(), top.end(), [&](int a, int b) { return B[a] != B[b] ? B[a] > B[b] : a < b; });
	if ((int)top.size() > top_m) top.resize(top_m);
	std::vector<std::pair<int, int>> sjobs;
	std::vector<Move> E2c, E2r;
	if (!eval_mode) {
		for (int r = 0; r < (int)Rr.size(); r++) for (int c : top) if ((int)sjobs.size() < smax) sjobs.push_back(std::make_pair(c, r));
	}
	else {
		for (const auto &pr : eval2) {
			if ((int)sjobs.size() >= smax) break;
			Move a = fallback, b = fallback;
			dop_set(a, pr.first);
			dop_set(b, pr.second);
			E2c.push_back(a);
			E2r.push_back(b);
			sjobs.push_back(std::make_pair((int)E2c.size() - 1, (int)E2r.size() - 1));
		}
	}
	const std::vector<Move> &SC = eval_mode ? E2c : C;
	const std::vector<Move> &SR = eval_mode ? E2r : Rr;
	const int nS = (int)sjobs.size();
	long ms2 = decap_ms_since(t0);
	int ran2 = dop_run(nS, [&](int k) {
		dop_state &S = st[k];
		S.phase = 1;
		S.c = sjobs[k].first;
		S.r = sjobs[k].second;
		struct timespec tj;
		clock_gettime(CLOCK_MONOTONIC, &tj);
		// [game access]
			if (decap_ms_since(t0) >= end_ms) return;
			dop_nap_us(500);
		}
		const Move &o = SC[S.c], &h = SR[S.r];
		{
			unsigned hs = plan_seed;
			for (int j = 0; j < 20; j++) hs = hs * 31u + o.states[j];
			for (int j = 0; j < 20; j++) hs = hs * 31u + h.states[j];
			// [game access]
		}
		decap_plan_turn_only = 1;
		S.o1 = decap_play_plan(me, o, h, frames0, dl);
		// [game access]
			S.over0 = 1;
			S.unsafe_all = S.o1.lost1 ? 1 : 0;
			S.t2 = 0.0f;
			// [game access]
			__sync_synchronize();
			S.phase = 2;
			return;
		}
		// [game access]
		Position rq = decap_position_for(me, dl, f1, true), rt = decap_position_for(op, dl, f1, true);
		std::vector<Move> F, G;
		std::vector<uint8_t> Fs, Gs;
		auto addF = [&](const Move &m, uint8_t src) {
			if ((int)F.size() >= fmax) return;
			for (const Move &q : F) if (memcmp(q.states, m.states, 20) == 0) return;
			F.push_back(m);
			Fs.push_back(src);
		};
		auto addG = [&](const Move &m, uint8_t src) {
			if ((int)G.size() >= ng_max) return;
			for (const Move &q : G) if (memcmp(q.states, m.states, 20) == 0) return;
			G.push_back(m);
			Gs.push_back(src);
		};
		// [game access]
		Move keep = base;
		memcpy(keep.states, o.states, 20);
		addF(keep, 0);
		addF(base, 1);
		{
			std::vector<Move> rel;
			dop_releases(keep, rel_mode, rel);
			for (const Move &m : rel) addF(m, 2);
		}
		// [game access]
		if (fknn > 0) {
			// [game access]
				memcpy(m.grip, base.grip, sizeof(m.grip));
				addF(m, 3);
			}
		}
		for (const Move &m : draws) addF(m, 4);
		{
			std::vector<Move> ke;
			decap_kick_edits(base, o.states, ke);
			for (size_t i = 0; i < draws.size() && i < 4; i++) decap_kick_edits(draws[i], o.states, ke);
			for (const Move &m : ke) addF(m, 5);
		}
		Move hk = base;
		memcpy(hk.states, h.states, 20);
		addG(hk, 0);
		if (glog > 0) {
			for (const std::string &m2 : dop_logged_second(opp, dop_str(h.states), glog)) {
				Move r = base;
				dop_set(r, m2);
				addG(r, 1);
			}
		}
		{
			int want = ng_max - 1 - (int)G.size();
			if (want > 0) {
				// [game access]
			}
			while ((int)G.size() > ng_max - 1) { G.pop_back(); Gs.pop_back(); }
			addG(hold, 3);
		}
		// [game access]
		const int nF = (int)F.size(), nG = (int)G.size();
		S.nF = nF;
		S.nG = nG;
		for (int i = 0; i < nF; i++) { memcpy(S.F[i], F[i].states, 20); S.Fsrc[i] = Fs[i]; }
		for (int i = 0; i < nG; i++) { memcpy(S.G[i], G[i].states, 20); S.Gsrc[i] = Gs[i]; }
		const int gg1 = std::min(g1, nG);
		std::vector<int> jl;
		for (int f = 0; f < nF; f++) for (int g = 0; g < gg1; g++) jl.push_back(f * nG + g);
		auto sim_fn = [&](int j) {
			int idx = jl[j], f = idx / nG, g = idx % nG;
			decap_plan_turn_only = 1;
			decap_outcome x = decap_play_plan(me, F[f], G[g], f1, dl);
			S.sim[idx].net = x.dealt1 - x.taken1;
			S.sim[idx].unsafe = dop_unsafe(x, limit, min_low) ? 1 : 0;
			S.sim[idx].lost = x.lost1 ? 1 : 0;
			__sync_synchronize();
			S.sim[idx].done = 1;
		};
		int ran = dop_run((int)jl.size(), sim_fn, jpar, &hdr->tok, t0, end_ms, alarm_s);
		S.sims = ran;
		if (ran < (int)jl.size()) return;
		auto score = [&](int f, int g_from, int g_to, bool *ok) {
			double s = 0.0;
			*ok = true;
			for (int g = g_from; g < g_to; g++) {
				const dop_sim &x = S.sim[f * nG + g];
				if (!x.done || x.unsafe) *ok = false;
				s += x.net;
			}
			return g_to > g_from ? s / (g_to - g_from) : 0.0;
		};
		std::vector<std::pair<double, int>> r1;
		for (int f = 0; f < nF; f++) {
			bool ok;
			double s = score(f, 0, gg1, &ok);
			if (ok) r1.push_back(std::make_pair(s, -f));
		}
		std::sort(r1.begin(), r1.end(), std::greater<std::pair<double, int>>());
		if ((int)r1.size() > k2) r1.resize(k2);
		S.k2 = (int)r1.size();
		jl.clear();
		for (const auto &x : r1) for (int g = gg1; g < nG; g++) jl.push_back(-x.second * nG + g);
		int ran2b = dop_run((int)jl.size(), sim_fn, jpar, &hdr->tok, t0, end_ms, alarm_s);
		S.sims += ran2b;
		if (ran2b < (int)jl.size()) return;
		int best = -1;
		double bs = -1e30;
		for (const auto &x : r1) {
			bool ok;
			double s = score(-x.second, 0, nG, &ok);
			if (ok && (s > bs || (s == bs && -x.second < best))) { bs = s; best = -x.second; }
		}
		S.best = best;
		S.unsafe_all = best < 0 ? 1 : 0;
		if (best >= 0) {
			double v = 0.0;
			for (int g = 0; g < nG; g++) v += (S.sim[best * nG + g].net - bs) * (S.sim[best * nG + g].net - bs);
			S.t2 = (float)bs;
			S.se2 = nG > 1 ? (float)sqrt(v / (nG - 1) / nG) : 0.0f;
		}
		S.ms = (int)decap_ms_since(tj);
		__sync_synchronize();
		S.phase = 2;
	}, eval_mode ? par : states_at_once, nullptr, t0, end_ms, alarm_s);
	ms2 = decap_ms_since(t0) - ms2;
	int done2 = 0, sims2 = 0;
	for (int k = 0; k < nS; k++) { done2 += st[k].phase == 2; sims2 += st[k].sims; }
	// [game access]
	if (eval_mode) {
		static const char *fsrc[6] = { "keep", "argmax", "release", "knn", "draw", "kick" };
		for (int k = 0; k < nS; k++) {
			const dop_state &S = st[k];
			const decap_outcome &o = S.o1;
			char b[1024];
			// [game access: 4 lines]
			dop_log(b);
		}
		cleanup();
		// [game access]
		return fallback;
	}

	const int nR = (int)Rr.size();
	double wR = 0.0;
	for (double w : Rw) wR += w;
	struct agg { int c; double tot, t1p, t2p, se, var, cov, rank; int nd; bool bad; };
	std::vector<agg> A;
	for (int c : top) {
		agg a = { c, 0, 0, 0, 0, 0, 0, 0, 0, false };
		double sw = 0.0, sw2 = 0.0, s1 = 0.0, s2 = 0.0;
		std::vector<std::pair<double, double>> xs;
		for (int k = 0; k < nS; k++) {
			if (sjobs[k].first != c || st[k].phase != 2) continue;
			const dop_state &S = st[k];
			if (S.unsafe_all) a.bad = true;
			double w = Rw[sjobs[k].second], t1 = S.o1.dealt1 - S.o1.taken1, t2 = S.t2;
			s1 += w * t1;
			s2 += w * t2;
			sw += w;
			sw2 += w * w * S.se2 * S.se2;
			xs.push_back(std::make_pair(w, t1 + t2));
			a.nd++;
		}
		if (sw <= 0) continue;
		a.t1p = s1 / sw;
		a.t2p = s2 / sw;
		a.tot = a.t1p + a.t2p;
		a.se = sqrt(sw2) / sw;
		for (const auto &x : xs) a.var += x.first * (x.second - a.tot) * (x.second - a.tot);
		a.var /= sw;
		a.cov = wR > 0 ? sw / wR : 0.0;
		A.push_back(a);
	}
	double covmax = 0.0;
	for (const agg &a : A) if (!a.bad) covmax = std::max(covmax, a.cov);
	int ib = -1;
	for (size_t i = 0; i < A.size(); i++) {
		agg &a = A[i];
		a.rank = a.tot;
		if (a.cov < covmax - 1e-9) a.rank -= lcb_k * sqrt((covmax - a.cov) * a.var + a.se * a.se);
		if (!a.bad && (ib < 0 || a.rank > A[ib].rank)) ib = (int)i;
	}
	// [game access]
	for (const agg &a : A) {
		// [game access]
	}
	for (int c = 0; c < nC; c++) {
		rec_c.push_back(C[c]);
		rec_tag.push_back(Ctag[c]);
		rec_safe.push_back((uint8_t)safe1[c]);
		rec_t1.push_back((float)T1[c]);
		float tot = NAN, rk = NAN;
		for (const agg &a : A) if (a.c == c) { tot = (float)a.tot; rk = (float)a.rank; }
		rec_tot.push_back(tot);
		rec_rank.push_back(rk);
	}
	DECAP_LF_OPEN_TOTALS();
	if (ib < 0) {
		cleanup();
		lb.add(" none usable");
		// [game access]
		return fallback;
	}
	const double best_rank = A[ib].rank, tau = std::max(tau_min, A[ib].se);
	std::vector<int> E;
	std::vector<double> P;
	double sp = 0.0;
	for (size_t i = 0; i < A.size(); i++) {
		if (A[i].bad || A[i].rank < best_rank - std::max(near_pts, margin * A[ib].se)) continue;
		E.push_back((int)i);
		P.push_back(near_pts > 0 ? 1.0 : exp((A[i].rank - best_rank) / tau));
		sp += P.back();
	}
	double u = rand() / ((double)RAND_MAX + 1.0), acc = 0.0;
	int pick = E.back();
	for (size_t k = 0; k < E.size(); k++) {
		acc += P[k] / sp;
		if (u < acc) { pick = E[k]; break; }
	}
	// [game access]
	if (near_pts <= 0) lb.add(" tau %.0f:", tau);
	for (size_t k = 0; k < E.size(); k++) lb.add(" %s p %.2f", dop_str(C[A[E[k]].c].states).c_str(), P[k] / sp);
	Move chosen = C[A[pick].c];
	// [game access]
	cleanup();
	DECAP_LF_OPEN_PICK();
	// [game access]
	return chosen;
}
