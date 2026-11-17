// decapAI bot, build B53: the full decision log: the record of every candidate, reply, score and stage of a decision.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

#pragma pack(push, 1)
struct decap_lf_hdr {
	uint32_t magic, size;
	uint16_t version, hdr_size;
	uint32_t n_sections;
	uint64_t ts_ns;
	int32_t tag, handle, player, slot, turn, pid;
};
struct decap_lf_sec { uint32_t id, size; };
struct decap_lf_meta {
	uint8_t final_stage;

	uint8_t level;
	uint8_t flags;

	uint8_t snaps;
	int32_t cem_note, legacy_played, legacy_n_cand;
	float legacy_score, d0;
	uint8_t played[N_JOINTS], played_grip[2];
	int32_t played_cand, played_fin;
	int32_t n_round1;
	int32_t lp_c0, lp_cn, lp_fin;
	int32_t ms_final, ms_write;
	int32_t capture_us, build_us;
	int32_t budget_ms, ko_full_ms, lp_ms;
	float pick_u;
	float xw_worst, anchor, tau, pick1_w, lp_play;
	float points_w, dm_w, dm_cont, carry_w, goal_scale, chase_w;
	float rg_w, rg_end_w, rg_min;
	int32_t rg_sum;
};
struct decap_lf_rep {
	uint8_t joints[N_JOINTS], grip[2];
	uint8_t src;

	uint8_t sets;

};
struct decap_lf_callrec {
	uint8_t phase;

	uint8_t flags;
	uint16_t n_moves, n_replies, pad;
	uint32_t answered;
	int32_t ms;
};
struct decap_lf_cand {
	uint8_t joints[N_JOINTS], grip[2];
	uint8_t tag;
	uint8_t origin;
	uint8_t safe;
	uint8_t n_plans;
	int16_t fin;
	uint16_t pad;
	uint32_t plan_first;
	float mean_v, min_v;
};
struct decap_lf_fin {
	int32_t cand;
	uint8_t safe[6];
	uint8_t flags;
	uint8_t pad;
	float logp;
	float score[6];
	float fmin[6];
	float fmean;
	int32_t fcnt;
	float rg_mean, rg_max;
	float lp_j;
};
struct decap_lf_plan {
	uint16_t rep;
	uint8_t call;
	uint8_t flags;

	uint8_t flags2;

	int8_t dm_op1, dm_me1, dm_op_c, dm_me_c;
	uint8_t timer1, low_cm, pad;
	float v;
	float value;
	float dealt1, taken1;
	float points;
	float rg1, rg_end;
};
struct decap_lf_ko_hdr {
	uint16_t hdr_size, move_size;
	int32_t choice, would;
	int32_t n_pool, n_rep, n_r1, r1_ran, n_surv, keep_eff, n_search, n_mut, n_rand, n_new;
	int32_t sims, noans, dispatchers, disp_fail, judged, rejected, passed, pick_rank, n_rank;
	int32_t veto_n, veto_first, fresh_n;
	int32_t ms_start, ms_end, ms_r1, ms_r2, ms_safe, ms_stage;
	float lcb_k, w_mean, w_se, w_lb, fresh_d, fresh_dse;
	float rg_min, rg_over, rg_pts;
	uint8_t flags;
	uint8_t pad[3];
	char status[48], decision[40];
};
struct decap_lf_ko_move {
	int16_t cand;
	uint8_t src;
	uint8_t flags;
	float s1, s2;
	float se;
	float risk;
};
struct decap_lf_open_hdr {
	uint16_t hdr_size, rep_size, cand_size, agg_size;
	uint8_t status, pad[3];
	int32_t n_cand, n_rep, n_top, n_elig, pick, opp_games;
	float lambda, cover, u, near_pts, margin, lcb_k, tau, best_rank, covmax;
	int32_t ms1, ms2, n1, ran1, n_states, states_done, sims2;
};
struct decap_lf_open_rep { uint8_t joints[N_JOINTS]; float w; };
struct decap_lf_open_cand { uint8_t joints[N_JOINTS]; uint8_t tag, safe1, pad[2]; float t1, prior, blend; };
struct decap_lf_open_agg { int32_t c, nd; float tot, t1p, t2p, se, var, cov, rank, p; uint8_t bad, elig, pad[2]; };
struct decap_lf_time { uint32_t magic, size; uint64_t ts_ns; int32_t pid; uint32_t build_us, write_us, bytes, open_us; };
struct decap_lf_commitrec {
	uint32_t magic, size;
	uint64_t ts_ns;
	int32_t pid, player, turn, ms, stage;
	uint8_t why, pad[3];
	float score;
};
#pragma pack(pop)

static bool decap_chase_state(int me, float *worst_w, float *chase_w);

struct decap_lf_t {
	bool on;
	int level, phase, calls, snaps;
	struct timespec t0;
	long long hook_ns;
	std::vector<decap_lf_rep> reps;
	std::vector<uint8_t> rep_phase;
	std::map<std::string, int> slot;
	std::vector<std::vector<decap_lf_plan>> plans;
	std::vector<decap_lf_callrec> call;
	std::vector<float> s_score[5], s_min[5];
	std::vector<uint8_t> s_safe[5];
	bool have_final, have_ko, have_open;
	decap_lf_meta meta;
	std::vector<decap_lf_cand> cand;
	std::vector<decap_lf_fin> fin;
	decap_lf_ko_hdr ko;
	std::vector<uint16_t> ko_rep;
	std::vector<decap_lf_ko_move> ko_pool;
	std::vector<uint8_t> ko_new;
	decap_lf_open_hdr op;
	std::vector<decap_lf_open_rep> op_rep;
	std::vector<decap_lf_open_cand> op_cand;
	std::vector<float> op_net;
	std::vector<uint8_t> op_fl;
	std::vector<decap_lf_open_agg> op_agg;
};
static decap_lf_t decap_lf;
static int decap_lf_fd = -1;
static pid_t decap_lf_for = 0;

static long long decap_lf_ns(const struct timespec &a)
{
	struct timespec b;
	clock_gettime(CLOCK_MONOTONIC, &b);
	return (long long)(b.tv_sec - a.tv_sec) * 1000000000ll + (b.tv_nsec - a.tv_nsec);
}

#pragma pack(push, 1)
struct decap_rr_lf_t {
	uint16_t hdr_size, version;
	float d0;
	int32_t ms_fallback;
	int32_t race_cands, race_results, race_safe;
};
#pragma pack(pop)
static struct { bool have; decap_rr_lf_t h; } decap_rr_lf;

static void decap_lf_begin()
{
	decap_en_lf.have = false;
	decap_rr_lf.have = false;
	decap_lf.level = decap_env_int("DECAP_LOG_FULL", 0);
	const char *dir = getenv("DECAP_DECISION_LOG");
	decap_lf.on = decap_lf.level > 0 && dir && *dir;
	if (!decap_lf.on) return;
	clock_gettime(CLOCK_MONOTONIC, &decap_lf.t0);
	decap_lf.phase = decap_lf.calls = decap_lf.snaps = 0;
	decap_lf.hook_ns = 0;
	decap_lf.reps.clear();
	decap_lf.rep_phase.clear();
	decap_lf.slot.clear();
	decap_lf.plans.clear();
	decap_lf.call.clear();
	for (int k = 0; k < 5; k++) {
		decap_lf.s_score[k].clear();
		decap_lf.s_min[k].clear();
		decap_lf.s_safe[k].clear();
	}
	decap_lf.have_final = decap_lf.have_ko = decap_lf.have_open = false;
	memset(&decap_lf.meta, 0, sizeof(decap_lf.meta));
	decap_lf.cand.clear();
	decap_lf.fin.clear();
	decap_lf.ko_rep.clear();
	decap_lf.ko_pool.clear();
	decap_lf.ko_new.clear();
	decap_lf.op_rep.clear();
	decap_lf.op_cand.clear();
	decap_lf.op_net.clear();
	decap_lf.op_fl.clear();
	decap_lf.op_agg.clear();
}

static int decap_lf_reply(const Move &r)
{
	for (size_t i = 0; i < decap_lf.reps.size(); i++) {
		if (decap_xp_same(decap_lf.reps[i].joints, decap_lf.reps[i].grip, r)) return (int)i;
	}
	decap_lf_rep x;
	memset(&x, 0, sizeof(x));
	memcpy(x.joints, r.states, N_JOINTS);
	x.grip[0] = r.grip[0];
	x.grip[1] = r.grip[1];
	x.src = 255;
	decap_lf.reps.push_back(x);
	decap_lf.rep_phase.push_back((uint8_t)decap_lf.phase);
	return (int)decap_lf.reps.size() - 1;
}

static void decap_lf_call(const std::vector<Move> &mine, const std::vector<Move> &theirs,
                          const std::vector<decap_outcome> &res, const std::vector<char> *mask, int limit, float min_low)
{
	if (!decap_lf.on || decap_plan_turn_only || decap_shadow_jobs || decap_plan_scripts) return;
	struct timespec ta;
	clock_gettime(CLOCK_MONOTONIC, &ta);
	const int nm = (int)mine.size(), nt = (int)theirs.size();
	std::vector<int> rep(nt);
	for (int t = 0; t < nt; t++) rep[t] = decap_lf_reply(theirs[t]);
	decap_lf.calls++;
	uint32_t answered = 0;
	for (int c = 0; c < nm; c++) {
		int slot = -1;
		for (int t = 0; t < nt; t++) {
			const size_t i = (size_t)c * nt + t;
			if ((mask && !(*mask)[i]) || i >= res.size()) continue;
			if (slot < 0) {
				auto ins = decap_lf.slot.insert(std::make_pair(std::string((const char *)mine[c].states, N_JOINTS), (int)decap_lf.plans.size()));
				if (ins.second) decap_lf.plans.emplace_back();
				slot = ins.first->second;
			}
			const decap_outcome &o = res[i];
			decap_lf_plan p;
			memset(&p, 0, sizeof(p));
			p.rep = (uint16_t)rep[t];
			p.call = (uint8_t)std::min(decap_lf.calls, 255);
			p.v = NAN;
			if (o.ok) {
				answered++;
				const bool uns = o.lost1 || (!o.over1 && (o.timer1 >= limit || o.low1 < min_low || o.near1));
				p.flags = (uint8_t)(1 | (o.lost1 ? 2 : 0) | (o.timer1 >= limit ? 4 : 0) | (o.low1 < min_low ? 8 : 0) | (o.over1 ? 16 : 0) |
				                    (o.lop1 ? 32 : 0) | (o.lme_plan ? 64 : 0) | (o.lop_plan ? 128 : 0));
				p.flags2 = (uint8_t)((o.near1 ? 1 : 0) | (o.carry1 ? 2 : 0) | (uns ? 4 : 0) | (o.shadow1 ? 8 : 0));
				p.dm_op1 = (int8_t)std::min(o.dm_op1, 127);
				p.dm_me1 = (int8_t)std::min(o.dm_me1, 127);
				p.dm_op_c = (int8_t)std::min(o.dm_op_c, 127);
				p.dm_me_c = (int8_t)std::min(o.dm_me_c, 127);
				p.timer1 = (uint8_t)std::min(std::max(o.timer1, 0), 255);
				p.low_cm = (uint8_t)std::min(255.0f, std::max(0.0f, o.low1 * 100.0f));
				p.v = i < decap_full.pv.size() ? decap_full.pv[i] : NAN;
				p.value = o.value;
				p.dealt1 = o.dealt1;
				p.taken1 = o.taken1;
				p.points = o.points;
				p.rg1 = o.rg1;
				p.rg_end = o.rg_end;
			}
			if (decap_lf.plans[slot].size() < 255) decap_lf.plans[slot].push_back(p);
		}
	}
	decap_lf_callrec k;
	memset(&k, 0, sizeof(k));
	k.phase = (uint8_t)decap_lf.phase;
	k.flags = (uint8_t)(mask ? 1 : 0);
	k.n_moves = (uint16_t)std::min(nm, 65535);
	k.n_replies = (uint16_t)std::min(nt, 65535);
	k.answered = answered;
	k.ms = (int32_t)(decap_lf_ns(decap_lf.t0) / 1000000);
	if (decap_lf.call.size() < 254) decap_lf.call.push_back(k);
	decap_lf.hook_ns += decap_lf_ns(ta);
}

static void decap_lf_snap(int k, const std::vector<float> &fscore, const std::vector<char> &fsafe, const std::vector<float> &fmin)
{
	if (!decap_lf.on || k < 0 || k > 4) return;
	struct timespec ta;
	clock_gettime(CLOCK_MONOTONIC, &ta);
	decap_lf.s_score[k] = fscore;
	decap_lf.s_min[k] = fmin;
	decap_lf.s_safe[k].assign(fsafe.begin(), fsafe.end());
	decap_lf.snaps |= 1 << k;
	decap_lf.phase = std::min(k + 1, 4);
	decap_lf.hook_ns += decap_lf_ns(ta);
}

struct decap_lf_final_in {
	int me, lp_fin, played_c, cem_note;
	const std::vector<Move> *cand, *theirs, *held, *adv;
	const std::vector<char> *c_safe, *fsafe;
	const std::vector<float> *c_mean, *c_min, *fscore, *fmin, *fmean;
	const std::vector<int> *fcnt;
	const std::vector<size_t> *fin;
	const std::vector<uint8_t> *tags;
	std::vector<float> logp, lp_j;
	size_t n_round1, lp_c0, lp_cn;
	bool rescue, ko_on, ko_changed, lp_bonus_ok;
	float xw_worst, anchor, tau, pick1_w, lp_play, d0, pick_u;
	long budget_ms, ko_full_ms, lp_ms;
};

static void decap_lf_final(const decap_lf_final_in &in)
{
	if (!decap_lf.on) return;
	struct timespec ta;
	clock_gettime(CLOCK_MONOTONIC, &ta);
	decap_lf_meta &m = decap_lf.meta;
	const std::vector<Move> &cand = *in.cand;
	const std::vector<size_t> &fin = *in.fin;
	m.final_stage = in.ko_changed ? 5 : (in.rescue ? 3 : (in.played_c >= 0 ? 4 : 1));
	m.flags = (uint8_t)((in.rescue ? 1 : 0) | (in.lp_bonus_ok ? 4 : 0) | (decap_rg_on() ? 8 : 0) | 32 | (in.ko_on ? 64 : 0));
	m.played_cand = in.played_c;
	m.played_fin = -1;
	m.n_round1 = (int32_t)in.n_round1;
	m.lp_c0 = (int32_t)in.lp_c0;
	m.lp_cn = (int32_t)in.lp_cn;
	m.lp_fin = in.lp_fin;
	m.ms_final = (int32_t)(decap_lf_ns(decap_lf.t0) / 1000000);
	m.budget_ms = (int32_t)std::min(in.budget_ms, 2000000000L);
	m.ko_full_ms = (int32_t)std::min(in.ko_full_ms, 2000000000L);
	m.lp_ms = (int32_t)std::min(in.lp_ms, 2000000000L);
	m.pick_u = in.pick_u;
	m.xw_worst = in.xw_worst;
	m.anchor = in.anchor;
	m.tau = in.tau;
	m.pick1_w = in.pick1_w;
	m.lp_play = in.lp_play;
	m.points_w = decap_env_int("DECAP_POINTS_WEIGHT_MILLI", 150) / 1000.0f;
	m.dm_w = decap_env_int("DECAP_DM_BONUS_MILLI", 0) / 1000.0f;
	m.dm_cont = decap_env_int("DECAP_DM_CONT_MILLI", 500) / 1000.0f;
	m.carry_w = decap_env_int("DECAP_CARRY_MILLI", 0) / 1000.0f;
	// [game access]
	{
		float ww, cw;
		m.chase_w = decap_chase_state(in.me, &ww, &cw) ? cw : 0.0f;
	}
	if (decap_rg_on()) {
		m.rg_w = decap_rg_weight(in.me);
		m.rg_end_w = decap_env_int("DECAP_RG_END_MILLI", 700) / 1000.0f;
		m.rg_min = decap_env_int("DECAP_RG_MIN_MILLI", 50) / 1000.0f;
		m.rg_sum = decap_env_int("DECAP_RG_SUM", 0);
	}
	std::vector<int> fin_of(cand.size(), -1);
	for (size_t k = 0; k < fin.size(); k++) if (fin[k] < cand.size()) fin_of[fin[k]] = (int)k;
	decap_lf.cand.clear();
	for (size_t c = 0; c < cand.size(); c++) {
		decap_lf_cand d;
		memset(&d, 0, sizeof(d));
		memcpy(d.joints, cand[c].states, N_JOINTS);
		d.grip[0] = cand[c].grip[0];
		d.grip[1] = cand[c].grip[1];
		const bool lp = c >= in.lp_c0 && c < in.lp_c0 + in.lp_cn;
		d.tag = lp ? 13 : (c < in.n_round1 && c < in.tags->size() ? (*in.tags)[c] : 8);
		d.origin = lp ? 2 : (c < in.n_round1 ? 0 : 1);
		d.mean_v = c < in.c_mean->size() ? (*in.c_mean)[c] : NAN;
		d.min_v = c < in.c_min->size() ? (*in.c_min)[c] : NAN;
		d.safe = c < in.c_safe->size() ? (in.rescue ? d.mean_v >= 1.0f : (*in.c_safe)[c] != 0) : 0;
		d.fin = (int16_t)fin_of[c];
		decap_lf.cand.push_back(d);
	}
	if (in.ko_changed && in.played_c >= 0 && (size_t)in.played_c == cand.size() - 1 && !std::isfinite(decap_lf.cand.back().mean_v)) {
		decap_lf.cand.back().origin = 3;
	}
	decap_lf.fin.clear();
	for (size_t k = 0; k < fin.size(); k++) {
		decap_lf_fin f;
		memset(&f, 0, sizeof(f));
		f.cand = (int32_t)fin[k];
		for (int s = 0; s < 5; s++) {
			const bool have = (decap_lf.snaps >> s) & 1 && k < decap_lf.s_score[s].size();
			f.score[s] = have ? decap_lf.s_score[s][k] : NAN;
			f.fmin[s] = have ? decap_lf.s_min[s][k] : NAN;
			f.safe[s] = have ? decap_lf.s_safe[s][k] : 255;
		}
		f.score[5] = (*in.fscore)[k];
		f.fmin[5] = (*in.fmin)[k];
		f.safe[5] = (*in.fsafe)[k] != 0;
		const bool lp = fin[k] >= in.lp_c0 && fin[k] < in.lp_c0 + in.lp_cn;
		f.flags = (uint8_t)(((int)fin[k] == in.played_c ? 1 : 0) | (lp ? 2 : 0) | ((int)k == in.lp_fin ? 4 : 0));
		if ((int)fin[k] == in.played_c) m.played_fin = (int32_t)k;
		f.logp = k < in.logp.size() ? in.logp[k] : NAN;
		f.fmean = (*in.fmean)[k];
		f.fcnt = (*in.fcnt)[k];
		if (decap_rg_on() && fin[k] < cand.size()) {
			f.rg_mean = decap_rg_mean(cand[fin[k]]);
			f.rg_max = decap_rg_max(cand[fin[k]]);
		}
		f.lp_j = k < in.lp_j.size() ? in.lp_j[k] : NAN;
		decap_lf.fin.push_back(f);
	}

	for (size_t i = 0; i < in.theirs->size(); i++) {
		int r = decap_lf_reply((*in.theirs)[i]);
		decap_lf.reps[r].sets |= 1 | (i == 0 ? 32 : 0);
	}
	for (const Move &q : *in.held) decap_lf.reps[decap_lf_reply(q)].sets |= 2;
	for (const Move &q : *in.adv) decap_lf.reps[decap_lf_reply(q)].sets |= 4;
	decap_lf.have_final = true;
	decap_lf.hook_ns += decap_lf_ns(ta);
}
#define DECAP_LF_FINAL() do { if (decap_lf.on) { \
	decap_lf_final_in lf_in; \
	lf_in.me = seat; lf_in.lp_fin = lp_pk; lf_in.played_c = played_c; lf_in.cem_note = cem_note; \
	lf_in.cand = &cand; lf_in.theirs = &theirs; lf_in.held = &held; lf_in.adv = &adv_seen; \
	lf_in.c_safe = &c_safe; lf_in.fsafe = &fsafe; lf_in.c_mean = &c_mean; lf_in.c_min = &c_min; \
	lf_in.fscore = &fscore; lf_in.fmin = &fmin; lf_in.fmean = &fmean; lf_in.fcnt = &fcnt; lf_in.fin = &fin; lf_in.tags = &utags; \
	for (size_t lf_k = 0; lf_k < fin.size(); lf_k++) { \
		lf_in.logp.push_back(logprob(cand[fin[lf_k]])); \
		lf_in.lp_j.push_back(lp_is(fin[lf_k]) && fin[lf_k] - lp_c0 < lp_kj.size() ? lp_kj[fin[lf_k] - lp_c0] : NAN); } \
	lf_in.n_round1 = mine.size(); lf_in.lp_c0 = lp_c0; lf_in.lp_cn = lp_cn; \
	lf_in.rescue = rescue_cem; lf_in.ko_on = ko_on; lf_in.ko_changed = ko_changed; lf_in.lp_bonus_ok = lp_bonus_ok; \
	lf_in.xw_worst = xw_worst; lf_in.anchor = anchor; lf_in.tau = tau; lf_in.pick1_w = pick1_w; lf_in.lp_play = lp_play; \
	lf_in.d0 = first.d0; lf_in.pick_u = (float)rg_u; lf_in.budget_ms = budget_ms; lf_in.ko_full_ms = ko_full_ms; lf_in.lp_ms = lp_ms; \
	decap_lf_final(lf_in); } } while (0)

struct decap_lf_ko_in {
	const std::vector<Move> *kp, *kr, *theirs;
	const std::vector<int> *kp_cand, *rgn;
	const std::vector<uint8_t> *kp_src;
	const std::vector<float> *s1, *s2, *pr;
	const std::vector<char> *kdq;
	const std::vector<double> *rgs;
	const std::vector<size_t> *surv, *rank2;
	decap_lf_ko_hdr h;
};

static void decap_lf_ko(decap_lf_ko_in &in, const char *status, const char *decision)
{
	if (!decap_lf.on) return;
	struct timespec ta;
	clock_gettime(CLOCK_MONOTONIC, &ta);
	decap_lf_ko_hdr &h = in.h;
	const int nr = h.n_rep, nr1 = h.n_r1;
	const size_t np = in.kp->size();
	h.n_pool = (int32_t)np;
	h.n_surv = (int32_t)in.surv->size();
	h.n_rank = (int32_t)in.rank2->size();
	// [game access: 2 lines]
	decap_lf.ko_rep.clear();
	Move hold = (*in.theirs)[0];
	for (int j = 0; j < N_JOINTS; j++) hold.states[j] = STATE_HOLD;
	for (const Move &r : *in.kr) {
		int i = decap_lf_reply(r);
		decap_lf.reps[i].sets |= 16 | (memcmp(r.states, hold.states, N_JOINTS) == 0 ? 64 : 0);
		decap_lf.ko_rep.push_back((uint16_t)i);
	}
	std::vector<uint8_t> fl(np, 0);
	for (size_t i : *in.surv) if (i < np) fl[i] |= 1;
	for (size_t i : *in.rank2) if (i < np) fl[i] |= 4;
	decap_lf.ko_pool.clear();
	decap_lf.ko_new.clear();
	h.n_new = 0;
	const float n_mean = np > 0 && !in.s2->empty() ? (*in.s2)[0] : NAN;
	for (size_t i = 0; i < np; i++) {
		decap_lf_ko_move x;
		memset(&x, 0, sizeof(x));
		x.cand = (int16_t)std::max(-1, std::min(32767, (*in.kp_cand)[i]));
		x.src = (*in.kp_src)[i];
		x.s1 = (*in.s1)[i];
		x.s2 = (*in.s2)[i];
		x.se = NAN;
		if (std::isfinite(x.s2) && nr > 1) {
			double mm = 0.0, vv = 0.0;
			for (int q = 0; q < nr; q++) mm += (*in.pr)[i * nr + q];
			mm /= nr;
			for (int q = 0; q < nr; q++) vv += ((*in.pr)[i * nr + q] - mm) * ((*in.pr)[i * nr + q] - mm);
			x.se = (float)sqrt(vv / (nr - 1) / nr);
			if (i != 0 && std::isfinite(n_mean) && (fl[i] & 4) && x.s2 - h.lcb_k * x.se > n_mean) fl[i] |= 8;
		}
		x.flags = (uint8_t)(fl[i] | ((*in.kdq)[i] ? 2 : 0));
		x.risk = (*in.rgn)[i] > 0 ? (float)((*in.rgs)[i] / (*in.rgn)[i]) : 0.0f;
		decap_lf.ko_pool.push_back(x);
		if (x.cand < 0) {
			decap_lf.ko_new.insert(decap_lf.ko_new.end(), (*in.kp)[i].states, (*in.kp)[i].states + N_JOINTS);
			h.n_new++;
		}
	}
	(void)nr1;
	decap_lf.ko = h;
	decap_lf.have_ko = true;
	decap_lf.hook_ns += decap_lf_ns(ta);
}
#define DECAP_LF_KO() do { if (decap_lf.on) { \
	decap_lf_ko_in lf_k; \
	memset(&lf_k.h, 0, sizeof(lf_k.h)); \
	lf_k.h.hdr_size = (uint16_t)sizeof(decap_lf_ko_hdr); lf_k.h.move_size = (uint16_t)sizeof(decap_lf_ko_move); \
	lf_k.kp = &kp; lf_k.kr = &kr; lf_k.theirs = &theirs; lf_k.kp_cand = &kp_cand; lf_k.rgn = &ko_rgn; lf_k.kp_src = &kp_src; \
	lf_k.s1 = &s1; lf_k.s2 = &s2; lf_k.pr = &ko_pr; lf_k.kdq = &kdq; lf_k.rgs = &ko_rgs; lf_k.surv = &surv; lf_k.rank2 = &rank2; \
	lf_k.h.choice = ko_choice; lf_k.h.would = ko_would; lf_k.h.n_rep = nr; lf_k.h.n_r1 = nr1; lf_k.h.r1_ran = (int32_t)ko_r1_ran; \
	lf_k.h.keep_eff = ko_keep_eff; lf_k.h.n_search = (int32_t)ko_n_search; lf_k.h.n_mut = ko_n_mut; lf_k.h.n_rand = ko_n_rand; \
	lf_k.h.sims = ko_sims; lf_k.h.noans = ko_noans; lf_k.h.dispatchers = ko_disp; lf_k.h.disp_fail = ko_disp_fail; \
	lf_k.h.judged = ko_judged; lf_k.h.rejected = ko_rejected; lf_k.h.passed = ko_passed; lf_k.h.pick_rank = ko_pick_rank; \
	lf_k.h.veto_n = ko_veto_n; lf_k.h.veto_first = ko_veto_first; lf_k.h.fresh_n = ko_fresh_n; \
	lf_k.h.ms_start = (int32_t)ko_t_start; lf_k.h.ms_end = (int32_t)std::min(ko_end, 2000000000L); lf_k.h.ms_r1 = (int32_t)ko_ms_r1; \
	lf_k.h.ms_r2 = (int32_t)ko_ms_r2; lf_k.h.ms_safe = (int32_t)ko_ms_safe; lf_k.h.ms_stage = (int32_t)(decap_ms_since(t0) - ko_t_start); \
	lf_k.h.lcb_k = (float)ko_lcb; lf_k.h.w_mean = (float)ko_w_mean; lf_k.h.w_se = (float)ko_w_se; lf_k.h.w_lb = (float)ko_w_lb; \
	lf_k.h.fresh_d = (float)ko_fresh_d; lf_k.h.fresh_dse = (float)ko_fresh_dse; \
	lf_k.h.rg_min = ko_rg_on ? ko_rg_min : 0.0f; lf_k.h.rg_over = ko_rg_on ? ko_rg_over : 0.0f; lf_k.h.rg_pts = ko_rg_pts; \
	lf_k.h.flags = (uint8_t)((ko_ok ? 1 : 0) | (ko_changed ? 2 : 0) | (ko_dry ? 4 : 0) | (ko_rg_on ? 8 : 0) | (ko_fin_forced ? 16 : 0)); \
	decap_lf_ko(lf_k, ko_status, ko_dec); } } while (0)

static void decap_lf_open_begin(int n_cand, int n_rep, int opp_games, float lambda, float cover, float near_pts, float margin, float lcb_k,
                                float covmax, int ms1, int ms2, int n1, int ran1, int n_states, int states_done, int sims2)
{
	// [REDACTED: this function mainly reads the game's state; 32 lines]
}
#define DECAP_LF_OPEN_TOTALS() do { if (decap_lf.on) { \
	decap_lf_open_begin(nC, nR, opp_n, (float)lam, (float)cum, (float)near_pts, (float)margin, (float)lcb_k, (float)covmax, (int)ms1, (int)ms2, n1, ran1, nS, done2, sims2); \
	for (int lf_r = 0; lf_r < nR; lf_r++) { decap_lf_open_rep lf_x; memcpy(lf_x.joints, Rr[lf_r].states, N_JOINTS); lf_x.w = (float)Rw[lf_r]; decap_lf.op_rep.push_back(lf_x); } \
	for (int lf_c = 0; lf_c < nC; lf_c++) { \
		decap_lf_open_cand lf_x; memset(&lf_x, 0, sizeof(lf_x)); memcpy(lf_x.joints, C[lf_c].states, N_JOINTS); lf_x.tag = Ctag[lf_c]; lf_x.safe1 = (uint8_t)safe1[lf_c]; \
		lf_x.t1 = (float)T1[lf_c]; lf_x.prior = (float)prior2[lf_c]; lf_x.blend = (float)B[lf_c]; decap_lf.op_cand.push_back(lf_x); \
		for (int lf_r = 0; lf_r < nR; lf_r++) { const dop_res1 &lf_y = res1[lf_c * nR + lf_r]; \
			decap_lf.op_net.push_back(lf_y.done ? lf_y.o.dealt1 - lf_y.o.taken1 : NAN); \
			decap_lf.op_fl.push_back((uint8_t)(lf_y.done ? (dop_unsafe(lf_y.o, limit, min_low) ? 3 : 1) : 0)); } } \
	for (size_t lf_i = 0; lf_i < A.size(); lf_i++) { \
		decap_lf_open_agg lf_x; memset(&lf_x, 0, sizeof(lf_x)); lf_x.c = A[lf_i].c; lf_x.nd = A[lf_i].nd; lf_x.tot = (float)A[lf_i].tot; lf_x.t1p = (float)A[lf_i].t1p; \
		lf_x.t2p = (float)A[lf_i].t2p; lf_x.se = (float)A[lf_i].se; lf_x.var = (float)A[lf_i].var; lf_x.cov = (float)A[lf_i].cov; lf_x.rank = (float)A[lf_i].rank; \
		lf_x.bad = A[lf_i].bad ? 1 : 0; decap_lf.op_agg.push_back(lf_x); } \
	decap_lf.op.n_top = (int32_t)A.size(); } } while (0)
#define DECAP_LF_OPEN_PICK() do { if (decap_lf.on && decap_lf.have_open) { \
	for (size_t lf_k = 0; lf_k < E.size(); lf_k++) if ((size_t)E[lf_k] < decap_lf.op_agg.size()) { decap_lf.op_agg[E[lf_k]].elig = 1; decap_lf.op_agg[E[lf_k]].p = (float)(P[lf_k] / sp); } \
	decap_lf.op.n_elig = (int32_t)E.size(); decap_lf.op.pick = pick; decap_lf.op.u = (float)u; decap_lf.op.best_rank = (float)best_rank; decap_lf.op.tau = (float)tau; } } while (0)

static int decap_lf_open()
{
	// [REDACTED: this function mainly reads the game's state; 30 lines]
}

static void decap_lf_preopen()
{
	if (decap_env_int("DECAP_LOG_FULL", 0) > 0) decap_lf_open();
}

static void decap_lf_write(const decap_dec_hdr &h)
{
	if (!decap_lf.on) return;
	struct timespec ta;
	clock_gettime(CLOCK_MONOTONIC, &ta);
	const int fd = decap_lf_open();
	if (fd < 0) return;
	const uint32_t open_us = (uint32_t)(decap_lf_ns(ta) / 1000);
	clock_gettime(CLOCK_MONOTONIC, &ta);
	decap_lf_meta &m = decap_lf.meta;
	const bool turn0 = (h.cem_note & (16384 | 32768)) && h.turn == 0 && !decap_lf.have_final;
	if (!decap_lf.have_final) m.final_stage = turn0 ? ((h.cem_note & 16384) ? 10 : 11) : 1;
	m.level = (uint8_t)std::min(decap_lf.level, 255);
	if (decap_lf.have_ko) m.flags |= 2;
	if (decap_xp.on) m.flags |= 16;
	m.snaps = (uint8_t)decap_lf.snaps;
	m.cem_note = h.cem_note;
	m.legacy_played = h.played;
	m.legacy_n_cand = h.n_cand;
	m.legacy_score = h.score;
	m.d0 = h.d0;
	memcpy(m.played, decap_last_dec.played.states, N_JOINTS);
	m.played_grip[0] = decap_last_dec.played.grip[0];
	m.played_grip[1] = decap_last_dec.played.grip[1];
	if (!decap_lf.have_final) m.played_cand = m.played_fin = m.lp_fin = -1;
	m.ms_write = (int32_t)(decap_lf_ns(decap_lf.t0) / 1000000);
	m.capture_us = (int32_t)(decap_lf.hook_ns / 1000);

	for (size_t i = 0; i < decap_lf.reps.size(); i++) {
		decap_lf_rep &r = decap_lf.reps[i];
		if (decap_lf.rep_phase[i] == 3) r.sets |= 8;
		if (!decap_xp.on) continue;
		for (const decap_xp_rep &x : decap_xp.reps) {
			if (memcmp(x.joints, r.joints, N_JOINTS) == 0 && (bool)x.grip[0] == (bool)r.grip[0] && (bool)x.grip[1] == (bool)r.grip[1]) {
				r.src = x.src;
				if (x.src == 7) r.sets |= 8;
				break;
			}
		}
	}

	if (!decap_lf.have_final && !turn0) {
		decap_lf.cand.assign(decap_lf.plans.size(), decap_lf_cand());
		for (const auto &kv : decap_lf.slot) {
			decap_lf_cand &d = decap_lf.cand[kv.second];
			memset(&d, 0, sizeof(d));
			memcpy(d.joints, kv.first.data(), N_JOINTS);
			d.grip[0] = m.played_grip[0];
			d.grip[1] = m.played_grip[1];
			d.tag = 255;
			d.fin = -1;
			d.mean_v = d.min_v = NAN;
			bool ok = !decap_lf.plans[kv.second].empty();
			for (const decap_lf_plan &p : decap_lf.plans[kv.second]) ok = ok && (p.flags & 1) && !(p.flags2 & 4);
			d.safe = ok;
			if (memcmp(d.joints, m.played, N_JOINTS) == 0) m.played_cand = kv.second;
		}
		for (const decap_dec_cand &c : decap_last_dec.cands) {
			auto it = decap_lf.slot.find(std::string((const char *)c.joints, N_JOINTS));
			if (it == decap_lf.slot.end()) continue;
			decap_lf.cand[it->second].tag = c.tag;
			decap_lf.cand[it->second].mean_v = c.mean_v;
			decap_lf.cand[it->second].min_v = c.min_v;
		}
	}

	std::vector<decap_lf_plan> rows;
	for (size_t c = 0; c < decap_lf.cand.size(); c++) {
		decap_lf_cand &d = decap_lf.cand[c];
		d.n_plans = 0;
		d.plan_first = (uint32_t)rows.size();
		if (!(decap_lf.level >= 2 || d.fin >= 0 || (int)c == m.played_cand)) continue;
		auto it = decap_lf.slot.find(std::string((const char *)d.joints, N_JOINTS));
		if (it == decap_lf.slot.end()) continue;
		const std::vector<decap_lf_plan> &ps = decap_lf.plans[it->second];
		d.n_plans = (uint8_t)std::min(ps.size(), (size_t)255);
		rows.insert(rows.end(), ps.begin(), ps.begin() + d.n_plans);
		if (d.fin >= 0 && (size_t)d.fin < decap_lf.fin.size()) {
			for (size_t q = 0; q < d.n_plans; q++) {
				if (ps[q].rep < decap_lf.reps.size() && (decap_lf.reps[ps[q].rep].sets & 8)) decap_lf.fin[d.fin].flags |= 8;
			}
		}
	}
	const uint16_t plan_size = (uint16_t)((m.flags & 8) ? sizeof(decap_lf_plan) : sizeof(decap_lf_plan) - 2 * sizeof(float));
	std::vector<char> buf;
	buf.reserve(sizeof(decap_lf_hdr) + 256 + decap_lf.reps.size() * sizeof(decap_lf_rep) + decap_lf.cand.size() * sizeof(decap_lf_cand) +
	            decap_lf.fin.size() * sizeof(decap_lf_fin) + rows.size() * plan_size + decap_lf.ko_pool.size() * sizeof(decap_lf_ko_move) +
	            decap_lf.ko_new.size() + 4096);
	auto put = [&](const void *p, size_t n) { if (n) buf.insert(buf.end(), (const char *)p, (const char *)p + n); };
	uint32_t n_sec = 0;
	size_t sec_at = 0;
	auto sec_begin = [&](uint32_t id) {
		decap_lf_sec s = { id, 0 };
		sec_at = buf.size();
		put(&s, sizeof(s));
		n_sec++;
	};
	auto sec_end = [&]() {
		uint32_t n = (uint32_t)(buf.size() - sec_at - sizeof(decap_lf_sec));
		memcpy(buf.data() + sec_at + 4, &n, 4);
	};
	decap_lf_hdr fh;
	memset(&fh, 0, sizeof(fh));
	fh.magic = 0x31465844u;
	fh.version = 1;
	fh.hdr_size = (uint16_t)sizeof(fh);
	fh.ts_ns = h.ts_ns;
	fh.tag = h.tag;
	fh.handle = h.handle;
	fh.player = h.player;
	fh.slot = h.slot;
	fh.turn = h.turn;
	// [game access]
	put(&fh, sizeof(fh));
	sec_begin(0x4154454Du);
	const size_t meta_at = buf.size();
	put(&m, sizeof(m));
	sec_end();

	auto rows_begin = [&](uint32_t id, size_t row_size, size_t n) {
		sec_begin(id);
		const uint16_t rs = (uint16_t)row_size, zero = 0;
		const uint32_t cnt = (uint32_t)n;
		put(&rs, 2);
		put(&zero, 2);
		put(&cnt, 4);
	};
	if (!decap_lf.reps.empty()) {
		rows_begin(0x53504552u, sizeof(decap_lf_rep), decap_lf.reps.size());
		put(decap_lf.reps.data(), decap_lf.reps.size() * sizeof(decap_lf_rep));
		sec_end();
	}
	if (!decap_lf.call.empty()) {
		rows_begin(0x4C4C4143u, sizeof(decap_lf_callrec), decap_lf.call.size());
		put(decap_lf.call.data(), decap_lf.call.size() * sizeof(decap_lf_callrec));
		sec_end();
	}
	if (!decap_lf.cand.empty()) {
		rows_begin(0x444E4143u, sizeof(decap_lf_cand), decap_lf.cand.size());
		put(decap_lf.cand.data(), decap_lf.cand.size() * sizeof(decap_lf_cand));
		sec_end();
	}
	if (!decap_lf.fin.empty()) {
		rows_begin(0x4C4E4946u, sizeof(decap_lf_fin), decap_lf.fin.size());
		put(decap_lf.fin.data(), decap_lf.fin.size() * sizeof(decap_lf_fin));
		sec_end();
	}
	if (!rows.empty()) {
		rows_begin(0x4E414C50u, plan_size, rows.size());
		for (const decap_lf_plan &p : rows) put(&p, plan_size);
		sec_end();
	}
	if (decap_lf.have_ko) {
		sec_begin(0x54534F4Bu);
		put(&decap_lf.ko, sizeof(decap_lf.ko));
		put(decap_lf.ko_rep.data(), decap_lf.ko_rep.size() * sizeof(uint16_t));
		put(decap_lf.ko_pool.data(), decap_lf.ko_pool.size() * sizeof(decap_lf_ko_move));
		put(decap_lf.ko_new.data(), decap_lf.ko_new.size());
		sec_end();
	}
	if (decap_lf.have_open) {
		sec_begin(0x4E45504Fu);
		put(&decap_lf.op, sizeof(decap_lf.op));
		put(decap_lf.op_rep.data(), decap_lf.op_rep.size() * sizeof(decap_lf_open_rep));
		put(decap_lf.op_cand.data(), decap_lf.op_cand.size() * sizeof(decap_lf_open_cand));
		put(decap_lf.op_net.data(), decap_lf.op_net.size() * sizeof(float));
		put(decap_lf.op_fl.data(), decap_lf.op_fl.size());
		put(decap_lf.op_agg.data(), decap_lf.op_agg.size() * sizeof(decap_lf_open_agg));
		sec_end();
	}
	if (decap_en_lf.have) {
		sec_begin(0x52544E45u);
		put(&decap_en_lf.h, sizeof(decap_en_lf.h));
		sec_end();
	}
	if (decap_rr_lf.have) {
		sec_begin(0x53455252u);
		put(&decap_rr_lf.h, sizeof(decap_rr_lf.h));
		sec_end();
	}
	const uint32_t end_mark = 0x444E4544u;
	put(&end_mark, 4);
	fh.size = (uint32_t)buf.size();
	fh.n_sections = n_sec;
	memcpy(buf.data(), &fh, sizeof(fh));
	const int32_t build_us = (int32_t)(decap_lf_ns(ta) / 1000);
	memcpy(buf.data() + meta_at + offsetof(decap_lf_meta, build_us), &build_us, 4);
	struct timespec tb;
	clock_gettime(CLOCK_MONOTONIC, &tb);
	ssize_t w = write(fd, buf.data(), buf.size());
	(void)w;
	if (decap_env_int("DECAP_LOG_FULL_TIME", 0)) {
		decap_lf_time t;
		memset(&t, 0, sizeof(t));
		t.magic = 0x31545844u;
		t.size = (uint32_t)sizeof(t);
		t.ts_ns = h.ts_ns;
		// [game access]
		t.build_us = (uint32_t)build_us;
		t.write_us = (uint32_t)(decap_lf_ns(tb) / 1000);
		t.bytes = (uint32_t)buf.size();
		t.open_us = open_us;
		ssize_t w2 = write(fd, &t, sizeof(t));
		(void)w2;
	}
	decap_lf.on = false;
}

static void decap_lf_commit(int player, long ms, int stage, const char *why, float score)
{
	// [REDACTED: this function mainly reads the game's state; 17 lines]
}
