// decapAI bot, build B53: the central search: which moves it tries, how it scores them and how it picks.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.
// Left out of this file: the hooks that connect the bot to the game, the test service, and everything about how
// a candidate is played forward.

#if BOT_BUILD

// [game access: 2 lines]
struct decap_outcome {
	int ok;
	int lost1;
	int timer1;
	float low1;
	int end_frame;
	float value;
	float d1;
	float v_toward1;
	float speed1;
	float ring1;
	int ready1;
	float points;
	int dm_op1;
	int dm_me1;
	int dm_op_c;
	int dm_me_c;
	int over1;
	float risk_op;
	float risk_me;
	int ext_me;
	int ext_op;
	int ext_frames;
	float dealt1, taken1;
	float dealt, taken;
	int lop1;
	int lme_plan, lop_plan;
	int shadow1;
	int carry1;
	int near1;
	int margin1;
	float d2, d3;
	float dealt2, dealt3;
	float rg1, rg_end;
	float air1;
	float high1;
};

static int decap_dqm_me = 0, decap_dqm_prev = 0, decap_dqm_margin = 99;
// [game access]
static void decap_dqm_hook()
{
	// [REDACTED: this function mainly reads the game's state; 7 lines]
}

static int decap_env_int(const char *name, int dflt)
{
	const char *v = getenv(name);
	return (v && *v) ? atoi(v) : dflt;
}

static float decap_ringout_risk(float edge, bool threatened)
{
	static const float lim[7] = { 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 4.0f };
	static const float thr[8] = { 0.534f, 0.263f, 0.133f, 0.060f, 0.035f, 0.016f, 0.006f, 0.001f };
	static const float safe[8] = { 0.076f, 0.107f, 0.080f, 0.057f, 0.036f, 0.018f, 0.013f, 0.004f };
	if (edge < 0.0f) return threatened ? 0.9f : 0.5f;
	int k = 0;
	while (k < 7 && edge >= lim[k]) k++;
	return threatened ? thr[k] : safe[k];
}

static void decap_ringout_risks(int me, float *risk_me, float *risk_op)
{
	// [REDACTED: this function mainly reads the game's state; 11 lines]
}

static float decap_floor_dq_risk(int timer)
{
	// [REDACTED: this function mainly reads the game's state; 4 lines]
}

static int decap_floor_risk_input(int p)
{
	// [game access: 2 lines]
}

static float decap_lowest_any(int p)
{
	// [REDACTED: this function mainly reads the game's state; 6 lines]
}
static float decap_lowest_part(int p)
{
	// [REDACTED: this function mainly reads the game's state; 11 lines]
}

static float decap_lowest_shape(int p)
{
	// [REDACTED: this function mainly reads the game's state; 28 lines]
}

static int decap_hp_me = 0;
static float decap_hp_peak = -1e9f;

static int decap_hp_logz = 0;
static double decap_hp_zc[2] = { 0.0, 0.0 };
static double decap_com_height(int p)
{
	// [REDACTED: this function mainly reads the game's state; 8 lines]
}
// [game access]
static void decap_hp_hook()
{
	// [REDACTED: this function mainly reads the game's state; 4 lines]
}

static double decap_com_vspeed(int p)
{
	// [REDACTED: this function mainly reads the game's state; 8 lines]
}
static float decap_apex_low(int p)
{
	// [REDACTED: this function mainly reads the game's state; 4 lines]
}

static bool decap_any_part_outside(int p)
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}

static bool decap_protect_on(int me, int frames)
{
	// [REDACTED: this function mainly reads the game's state; 4 lines]
}

static void decap_set_moves(int p, const Move &move)
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}

// [game access: 2 lines]
{
	// [REDACTED: this function mainly reads the game's state; 41 lines]
}

struct decap_goal_sit {
	float d0, chase, run;
	int near, opp_launching, ready0;
};

static bool decap_launch_ready(int p);

static void decap_mean_vel(int p, float v[3])
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}

static float decap_chest_dist(int a, int b, float u[2])
{
	// [REDACTED: this function mainly reads the game's state; 6 lines]
}

static decap_goal_sit decap_goal_situation(int me)
{
	// [REDACTED: this function mainly reads the game's state; 17 lines]
}

static void decap_goal_measure(int me, decap_outcome &o)
{
	// [REDACTED: this function mainly reads the game's state; 10 lines]
}

static bool decap_rg_on()
{
	return decap_env_int("DECAP_RINGGUARD", 0) != 0;
}

static float decap_rg_risk(int p)
{
	// [REDACTED: this function mainly reads the game's state; 20 lines]
}

struct decap_rg_stat { double sum; int n; float max; };
static std::map<std::string, decap_rg_stat> decap_rg_seen;
static void decap_rg_note(const Move &m, float rg)
{
	decap_rg_stat &s = decap_rg_seen[std::string((const char *)m.states, N_JOINTS)];
	s.sum += rg;
	s.n++;
	if (rg > s.max) s.max = rg;
}
static float decap_rg_mean(const Move &m)
{
	auto it = decap_rg_seen.find(std::string((const char *)m.states, N_JOINTS));
	return it == decap_rg_seen.end() || it->second.n == 0 ? 0.0f : (float)(it->second.sum / it->second.n);
}
static float decap_rg_max(const Move &m)
{
	auto it = decap_rg_seen.find(std::string((const char *)m.states, N_JOINTS));
	return it == decap_rg_seen.end() || it->second.n == 0 ? 0.0f : it->second.max;
}

static float decap_rg_weight(int me)
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}

static float decap_hit_chance(float d)
{
	static const float at[6] = { 1.4f, 1.55f, 1.85f, 2.15f, 2.45f, 3.0f };
	static const float p[6] = { 0.70f, 0.55f, 0.40f, 0.25f, 0.15f, 0.05f };
	if (!std::isfinite(d)) return p[5];
	if (!(d < at[5])) return p[5] - 0.15f * (d - at[5]);
	if (d <= at[0]) return p[0];
	int k = 1;
	while (d > at[k]) k++;
	return p[k - 1] + (p[k] - p[k - 1]) * (d - at[k - 1]) / (at[k] - at[k - 1]);
}

static float decap_arrival(const decap_outcome &o, float g, float d0)
{
	float p0 = decap_hit_chance(d0);
	float a = decap_hit_chance(o.d1) - p0;
	if (std::isfinite(o.d2)) a = std::max(a, g * (decap_hit_chance(o.d2) - p0));
	if (std::isfinite(o.d3)) a = std::max(a, g * g * (decap_hit_chance(o.d3) - p0));
	if (o.dealt1 >= 1000.0f) a = std::max(a, 0.0f);
	return a;
}

static float decap_goal_bonus(const decap_goal_sit &s, const decap_outcome &o, float scale, float arrive_w, float arrive_g)
{
	float b = -0.03f * std::min(o.timer1, 20) / 20.0f;
	if (s.near) {
		return b * scale;
	}
	float even = std::max(0.0f, 1.0f - s.chase - s.run);
	if (arrive_w > 0.0f) {
		b += 0.15f * arrive_w * (s.chase + 0.3f * even) * decap_arrival(o, arrive_g, s.d0) / 0.70f;
	}
	else {
		b += 0.15f * (s.chase + 0.3f * even) * tanhf((s.d0 - o.d1) / 0.75f);
	}
	if (s.ready0) {
		if (!(arrive_w > 0.0f)) b += 0.10f * (s.chase + 0.3f * even) * tanhf(std::max(0.0f, o.v_toward1) / 6.0f);
	}
	else {
		b += 0.05f * (1.0f - s.run) * o.ready1;
	}
	if (s.run > 0.0f) {
		if (s.opp_launching) {
			b += 0.15f * s.run * tanhf((o.d1 - s.d0) / 0.75f);
		}
		else {
			b += 0.05f * s.run * (o.ready1 + 1.0f - tanhf(o.speed1 / 2.0f));
		}
	}
	b -= 0.20f * o.ring1;
	return b * scale;
}

struct decap_script {
	Move first;
	std::vector<Move> rest;
};
static const std::vector<decap_script> &decap_jump_templates()
{
	static std::vector<decap_script> cache;
	static bool loaded = false;
	if (loaded) return cache;
	loaded = true;
	const char *path = getenv("DECAP_JUMP_FILE");
	FILE *f = path ? fopen(path, "r") : nullptr;
	if (!f) return cache;
	char line[4096];
	Move relax = {};
	for (int j = 0; j < N_JOINTS; j++) relax.states[j] = 4;
	while (fgets(line, sizeof(line), f)) {
		std::vector<Move> seq;
		// [game access]
			if (tok[0] == '#' || strlen(tok) < (size_t)N_JOINTS) break;
			Move r = {};
			for (int j = 0; j < N_JOINTS; j++) r.states[j] = (uint8_t)(tok[j] - '0');
			seq.push_back(r);
		}
		if (seq.empty()) continue;
		decap_script s;
		s.first = seq[0];
		s.rest.assign(seq.begin() + 1, seq.end());
		while (s.rest.size() < 6) s.rest.push_back(relax);
		cache.push_back(s);
	}
	fclose(f);
	return cache;
}

static std::vector<decap_script> decap_decision_scripts;
static const std::vector<Move> *decap_script_for(const Move &mine)
{
	for (const decap_script &s : decap_jump_templates()) {
		if (memcmp(s.first.states, mine.states, sizeof(mine.states)) == 0) return &s.rest;
	}
	for (const decap_script &s : decap_decision_scripts) {
		if (memcmp(s.first.states, mine.states, sizeof(mine.states)) == 0) return &s.rest;
	}
	return nullptr;
}

static int decap_plan_turn_only = 0;
static const std::vector<char> *decap_plan_mask = nullptr;
static long decap_plan_budget_ms = -1;

static const std::vector<std::vector<Move>> *decap_plan_scripts = nullptr;
static const std::vector<Move> *decap_plan_script = nullptr;
static std::vector<decap_outcome> *decap_plan_outcomes = nullptr;
static int decap_plan_opp_keep = 0;

struct decap_shadow_job {
	Move bot[3], opp[3];
	int8_t subj;
	int8_t score;
	int8_t keep_cap, keep_pairs;
};
struct decap_shadow_meta {
	int32_t ok, unknown, nf[3], tf[3], lme[3], lop[3], over[3], ran[3];
	double V, reach, close, neg, J, touch[3], predD[3];
	int32_t fl, pad;
	double pts[2];
};
static const std::vector<decap_shadow_job> *decap_shadow_jobs = nullptr;
static char *decap_shadow_shm = nullptr;
static size_t decap_shadow_slot = 0;
// [game access: 2 lines]
static int decap_shadow_freeze = 0;

static float decap_opp_obj(const decap_outcome &o, int limit, float min_low, float loss_pts, float unsafe_pts, float dm_pts,
                           float dealt_w)
{
	float j = o.taken1 - dealt_w * o.dealt1;
	bool unsafe = !o.over1 && (o.timer1 >= limit || o.low1 < min_low || o.near1);
	if (o.lost1) j += loss_pts;
	else if (unsafe) j += unsafe_pts;
	j += dm_pts * (float)(o.dm_me1 - o.dm_op1);
	if (o.lop1) j -= loss_pts;
	return j;
}

static void decap_cs_log(const char *line)
{
	// [REDACTED: this function mainly reads the game's state; 9 lines]
}

#include "decap_speed.inc"
// [game access]
static decap_outcome decap_play_plan(int me, const Move &mine, const Move &theirs, int frames,
                                     float skill_level)
{
	// [REDACTED: this function mainly reads the game's state; 269 lines]
}

// [game access]
static double *decap_sh_cap = nullptr;
static int decap_sh_subj = -1, decap_sh_t0 = 0, decap_sh_nf = 0, decap_sh_long = 0;

static void decap_sh_capture_frame(int p, double *o)
{
	// [REDACTED: this function mainly reads the game's state; 41 lines]
}

static void decap_sh_hook()
{
	// [REDACTED: this function mainly reads the game's state; 6 lines]
}

static int decap_sh_unknown_shapes()
{
	// [REDACTED: this function mainly reads the game's state; 13 lines]
}

static bool decap_sh_cross_grip()
{
	// [REDACTED: this function mainly reads the game's state; 12 lines]
}

static decap_outcome decap_shadow_play(int me, int idx, int frames, int freeze)
{
	// [REDACTED: this function mainly reads the game's state; 105 lines]
}

static double decap_sh_hc_bot(double d) { return std::isfinite(d) ? (double)decap_hit_chance((float)d) : 0.0; }

static void decap_sh_log(const char *line)
{
	// [REDACTED: this function mainly reads the game's state; 12 lines]
}
// [game access: 2 lines]
static long decap_ms_since(const struct timespec &t0)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_sec - t0.tv_sec) * 1000 + (t.tv_nsec - t0.tv_nsec) / 1000000;
}

struct decap_pick {
	int candidate;
	int safe;
	int results;
	float score;
	float best;
	float plain;
};

enum { DECAP_DIAG_MAX = 64 };
struct decap_diag_t {
	int32_t n, played, ready;
	float d0;
	uint8_t tag[DECAP_DIAG_MAX], why[DECAP_DIAG_MAX];
	float score[DECAP_DIAG_MAX], v_toward[DECAP_DIAG_MAX], low[DECAP_DIAG_MAX], value[DECAP_DIAG_MAX];
	int32_t no_answer;
	float mean_v[DECAP_DIAG_MAX], min_v[DECAP_DIAG_MAX];
};
static decap_diag_t decap_diag;

struct decap_full_t {
	std::vector<uint8_t> why;
	std::vector<float> mean_v, min_v, value;
	std::vector<float> mean_all, min_all;
	std::vector<int> n_ans, surv;
	std::vector<int> dq_frame;

	int nt = 0;
	std::vector<float> pv, pj, pd;
	std::vector<uint8_t> pu;
	bool ok(size_t c) const { return c < why.size() && why[c] == 0 && std::isfinite(mean_v[c]); }
};
static decap_full_t decap_full;

struct decap_dec_cand { uint8_t joints[N_JOINTS]; uint8_t grip[2]; uint8_t safe, tag; float mean_v, min_v, fscore; };
struct decap_dec_hdr {
	uint32_t magic, size;
	uint64_t ts_ns;
	int32_t tag, handle, player, slot, turn, n_cand, played, cem_note, ready, req_size, cand_size;
	float score, d0;
};
static struct {
	Position req;
	std::vector<decap_dec_cand> cands;
	Move played;
	int32_t played_idx, cem_note, ready;
	float d0, score;
	bool valid;
} decap_last_dec;

struct decap_xp_rep { uint8_t joints[N_JOINTS]; uint8_t grip[2]; uint8_t src, pad; };
struct decap_xp_row_hdr { uint8_t joints[N_JOINTS]; uint8_t grip[2]; uint8_t role, n_out; };
struct decap_xp_out { uint16_t rep; uint8_t flags, call; int8_t dm_op1, dm_me1; uint8_t pad[2]; float v, value, dealt1, taken1, dealt, taken; };
struct decap_xp_t {
	bool on;
	int calls;
	std::vector<decap_xp_rep> reps;
	std::vector<Move> cand;
	std::vector<std::vector<decap_xp_out>> outs;
	std::vector<Move> attacked;
};
static decap_xp_t decap_xp;

struct decap_xp_counter {
	uint32_t magic;
	int16_t status, rounds;
	uint16_t attacked, turn_sims, full_plans, answered, kept, changed;
	int32_t ms_start, ms_used;
	float d0, guess_taken, counter_taken, guess_j, counter_j, fs_before, fs_after, mean_delta;
};
static decap_xp_counter decap_cs;
template <typename G> static bool decap_xp_same(const uint8_t *joints, const G *grip, const Move &r)
{
	return memcmp(joints, r.states, N_JOINTS) == 0 && (bool)grip[0] == r.grip[0] && (bool)grip[1] == r.grip[1];
}
static void decap_xp_reset()
{
	// [REDACTED: this function mainly reads the game's state; 7 lines]
}
static int decap_xp_reply(const Move &r, uint8_t src)
{
	// [REDACTED: this function mainly reads the game's state; 11 lines]
}
static void decap_xp_label(const std::vector<Move> &rs, size_t from, uint8_t src)
{
	if (!decap_xp.on) return;
	for (size_t i = from; i < rs.size(); i++) decap_xp_reply(rs[i], src);
}
static int decap_xp_slot(const Move &c)
{
	// [REDACTED: this function mainly reads the game's state; 6 lines]
}
static void decap_xp_add(int slot, int rep, const decap_outcome &o, float v, uint8_t flags)
{
	// [REDACTED: this function mainly reads the game's state; 14 lines]
}

static std::vector<char> decap_xp_trailer()
{
	// [REDACTED: this function mainly reads the game's state; 59 lines]
}

struct decap_ponder_slot { volatile int32_t stage, done; float score; Move best; };
static decap_ponder_slot *decap_publish_to = nullptr;
static void decap_publish(const Move &r, float score, int stage)
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}
static int decap_dec_fd = -1;
static pid_t decap_dec_pid = 0;

static int decap_decision_log_open()
{
	// [REDACTED: this function mainly reads the game's state; 12 lines]
}
// [game access]
static void decap_decision_log(int32_t tag, int32_t handle, int32_t player, int32_t slot, int32_t turn)
{
	// [REDACTED: this function mainly reads the game's state; 43 lines]
}

static bool decap_chase_state(int me, float *worst_w, float *chase_w)
{
	// [REDACTED: this function mainly reads the game's state; 10 lines]
}

static float decap_root_value = -1.0f;

static bool decap_rescue_better(size_t a, size_t b)
{
	const decap_full_t &f = decap_full;
	if (f.surv[a] != f.surv[b]) return f.surv[a] > f.surv[b];
	if (f.dq_frame[a] != f.dq_frame[b]) return f.dq_frame[a] > f.dq_frame[b];
	float ma = std::isfinite(f.mean_all[a]) ? f.mean_all[a] : -1e9f, mb = std::isfinite(f.mean_all[b]) ? f.mean_all[b] : -1e9f;
	return ma > mb;
}

static float decap_rescue_score(const decap_full_t &f, size_t c, int nt, int start, int frames)
{
	if (c >= f.mean_all.size() || !std::isfinite(f.mean_all[c])) return -9.0f;
	float late = f.dq_frame[c] >= (1 << 30) ? 1.0f : std::min(1.0f, std::max(0.0f, (f.dq_frame[c] - start) / (float)(std::max(1, frames) + 1)));
	return f.surv[c] / (float)std::max(1, nt) + 0.1f * late + 0.001f * tanhf(f.mean_all[c]);
}

static decap_pick decap_lookahead(int me, const std::vector<Move> &mine, const std::vector<Move> &theirs,
                                  int frames, float skill_level, unsigned plan_seed)
{
	decap_pick pick = { 0, 0, 0, -1.0f, -1.0f, -1.0f };
	int nm = (int)mine.size(), nt = (int)theirs.size(), n = nm * nt;
	decap_full.why.assign(nm, 4);
	decap_full.mean_v.assign(nm, NAN);
	decap_full.min_v.assign(nm, NAN);
	decap_full.value.assign(nm, NAN);
	decap_full.mean_all.assign(nm, NAN);
	decap_full.min_all.assign(nm, NAN);
	decap_full.n_ans.assign(nm, 0);
	decap_full.surv.assign(nm, 0);
	decap_full.dq_frame.assign(nm, 1 << 30);
	decap_full.nt = nt;
	decap_full.pv.assign(n, NAN);
	decap_full.pj.assign(n, NAN);
	decap_full.pd.assign(n, NAN);
	decap_full.pu.assign(n, 0);
	const std::vector<char> *mask = decap_plan_mask && (int)decap_plan_mask->size() == n ? decap_plan_mask : nullptr;
	int n_want = n;
	if (mask) {
		n_want = 0;
		for (int i = 0; i < n; i++) n_want += (*mask)[i] != 0;
	}
	std::vector<decap_outcome> res(n);
	std::vector<int> fds(n, -1);
	std::vector<pid_t> pids(n, -1);
	int parallel = decap_env_int("DECAP_LOOKAHEAD_PARALLEL", n);
	int budget = decap_env_int("DECAP_LOOKAHEAD_MS", 3000);
	unsigned alarm_s = (unsigned)std::max(1, decap_env_int("DECAP_PLAN_ALARM_S", std::max(3, budget / 1000 + 1)));
	if (decap_plan_budget_ms >= 0 && decap_plan_budget_ms < budget) budget = (int)decap_plan_budget_ms;
	// [game access]
	struct timespec t0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	int next = 0, running = 0;

	const bool t1c = decap_t1_active() && !decap_shadow_jobs;
	std::vector<decap_outcome> t1v;
	std::vector<char> t1vf;
	if (t1c) {
		t1v.resize(n);
		t1vf.assign(n, 0);
	}
	while (next < n || running > 0) {
		while (next < n && running < parallel) {
			if (mask && !(*mask)[next]) {
				next++;
				continue;
			}
			if (decap_plan_budget_ms >= 0 && decap_ms_since(t0) >= budget) break;
			if (t1c && decap_plan_turn_only && decap_t1_active()) {
				auto it = decap_t1_cache.find(decap_t1_key(mine[next / nt], theirs[next % nt], frames, me));
				if (it != decap_t1_cache.end()) {
					decap_t1_stats.hits++;
					const bool check = decap_t1_mode == 2 || (decap_t1_every > 0 && decap_t1_stats.hits % decap_t1_every == 0);
					if (!check) {
						res[next] = it->second;
						pick.results++;
						decap_t1_stats.served++;
						next++;
						continue;
					}
					t1v[next] = it->second;
					t1vf[next] = 1;
				}
			}
			int pfd[2];
			// [game access]
				next++;
				continue;
			}
			// [game access]
			if (pid == 0) {
				close(pfd[0]);
				// [game access: 3 lines]
				if (decap_plan_scripts && (size_t)(next / nt) < decap_plan_scripts->size() && !(*decap_plan_scripts)[next / nt].empty()) {
					decap_plan_script = &(*decap_plan_scripts)[next / nt];
				}
				if (decap_shadow_jobs && (size_t)next < decap_shadow_jobs->size()) {
					decap_outcome so = decap_shadow_play(me, next, frames, decap_shadow_freeze);
					ssize_t sw = write(pfd[1], &so, sizeof(so));
					(void)sw;
					// [game access]
				}
				if (t1c && !decap_plan_turn_only) {
					decap_t1_emit = 1;
					decap_t1_have = 0;
					decap_outcome pr[2];
					pr[0] = decap_play_plan(me, mine[next / nt], theirs[next % nt], frames, skill_level);
					if (decap_t1_have) pr[1] = decap_t1_out;
					else memset(&pr[1], 0, sizeof(pr[1]));
					ssize_t w2 = write(pfd[1], pr, sizeof(pr));
					(void)w2;
					// [game access]
				}
				decap_outcome o = decap_play_plan(me, mine[next / nt], theirs[next % nt], frames, skill_level);
				ssize_t wrote = write(pfd[1], &o, sizeof(o));
				(void)wrote;
				// [game access]
			}
			close(pfd[1]);
			if (pid < 0) {
				close(pfd[0]);
				next++;
				continue;
			}
			fds[next] = pfd[0];
			pids[next] = pid;
			running++;
			next++;
		}
		if (running == 0) {
			break;
		}
		long left = budget - decap_ms_since(t0);
		if (left <= 0) {
			break;
		}
		// [game access]
		std::vector<int> which;
		for (int i = 0; i < next; i++) {
			if (fds[i] >= 0) {
				// [game access]
				pf.push_back(p);
				which.push_back(i);
			}
		}
		// [game access]
		if (r <= 0) {
			break;
		}
		for (size_t k = 0; k < pf.size(); k++) {
			if (!pf[k].revents) {
				continue;
			}
			int i = which[k];
			decap_outcome o;
			if (t1c && !decap_plan_turn_only) {
				decap_outcome pr[2];
				if (read(fds[i], pr, sizeof(pr)) == (ssize_t)sizeof(pr) && pr[0].ok == 1) {
					res[i] = pr[0];
					pick.results++;
					if (pr[1].ok == 1) {
						decap_t1_stats.emitted++;
						decap_t1_put(decap_t1_key(mine[i / nt], theirs[i % nt], frames, me), pr[1]);
					}
				}
			}
			else if (read(fds[i], &o, sizeof(o)) == (ssize_t)sizeof(o) && o.ok == 1) {
				res[i] = o;
				pick.results++;
				if (t1c && decap_plan_turn_only) {
					if (t1vf[i]) decap_t1_check(o, t1v[i]);
					else decap_t1_put(decap_t1_key(mine[i / nt], theirs[i % nt], frames, me), o);
				}
			}
			close(fds[i]);
			fds[i] = -1;
			// [game access]
			pids[i] = -1;
			running--;
		}
	}
	for (int i = 0; i < n; i++) {
		if (pids[i] > 0) {
			// [game access: 2 lines]
		}
		if (fds[i] >= 0) {
			close(fds[i]);
		}
	}
	if (t1c) decap_t1_log_call();
	if (decap_plan_outcomes) *decap_plan_outcomes = res;
	decap_diag.no_answer = n_want - pick.results;
	if (pick.results == 0) {
		return pick;
	}
	int limit = decap_env_int("DECAP_TIMER_LIMIT", 10);
	float min_low = (decap_protect_on(me, frames) ? decap_env_int("DECAP_PROTECT_MIN_LOW_MILLI", 250)
	                                              : decap_env_int("DECAP_MIN_LOW_MILLI", 150)) / 1000.0f;

	const float oj_loss = (float)decap_env_int("DECAP_OPP_CEM_LOSS_PTS", 100000), oj_unsafe = (float)decap_env_int("DECAP_OPP_CEM_UNSAFE_PTS", 30000);
	const float oj_dm = (float)decap_env_int("DECAP_OPP_CEM_DM_PTS", 20000), oj_dealt = decap_env_int("DECAP_OPP_CEM_DEALT_MILLI", 500) / 1000.0f;

	float points_w = decap_env_int("DECAP_POINTS_WEIGHT_MILLI", 150) / 1000.0f;

	float urgency = 1.0f;
	if (decap_env_int("DECAP_POINTS_URGENCY", 0)) {
		// [game access]
		urgency = std::min(5.0f, 15.0f / (float)tl);
	}
	float dm_w = decap_env_int("DECAP_DM_BONUS_MILLI", 0) / 1000.0f;
	float dm_cont = decap_env_int("DECAP_DM_CONT_MILLI", 500) / 1000.0f;
	float carry_w = decap_env_int("DECAP_CARRY_MILLI", 0) / 1000.0f;
	float chase_hit = (float)decap_env_int("DECAP_CHASE_HIT_PTS", 0);
	float chase_near = decap_env_int("DECAP_CHASE_NEAR_MILLI", 0) / 1000.0f;

	bool desperate = false, defend = false;
	float press_w = 0.0f;
	float edge_w = decap_env_int("DECAP_EDGE_RISK_MILLI", 0) / 1000.0f;
	float press_edge = decap_env_int("DECAP_EDGE_PRESS_MILLI", 0) / 1000.0f;
	if (decap_env_int("DECAP_DESPERATE", 0)) {
		// [game access: 2 lines]
		float need = (float)decap_env_int("DECAP_DESPERATE_DEFICIT", 20000);
		bool late = tl <= decap_env_int("DECAP_DESPERATE_TURNS", 6);

		int dmode = decap_env_int("DECAP_DESPERATE", 0);
		desperate = late && deficit >= need && dmode != 3;
		defend = late && -deficit >= decap_env_int("DECAP_DEFEND_LEAD", (int)need) && dmode != 2;
		press_w = decap_env_int("DECAP_PRESS_MILLI", 1000) / 1000.0f;
		if (desperate) {
			points_w = 0.0f;
			dm_w = std::max(dm_w, decap_env_int("DECAP_DESPERATE_DM_MILLI", 280) / 1000.0f);
		}
		if (defend && decap_env_int("DECAP_DEFEND_POINTS_OFF", 0)) {
			points_w = 0.0f;
		}
	}

	int goals = decap_env_int("DECAP_GOALS", 0);
	float goal_scale = decap_env_int("DECAP_GOAL_SCALE_MILLI", 1000) / 1000.0f;
	float arrive_w = decap_env_int("DECAP_ARRIVE_MILLI", 0) / 1000.0f;
	float arrive_g = decap_env_int("DECAP_ARRIVE_GAMMA_MILLI", 800) / 1000.0f;
	decap_goal_sit sit = decap_goal_situation(me);
	const float air1_w = decap_env_int("DECAP_AIR1_MILLI", 0) / 1000.0f;
	const float air1_h = decap_env_int("DECAP_AIR1_MM", 50) / 1000.0f;
	const float high1_w = decap_env_int("DECAP_HIGH1_MILLI", 0) / 1000.0f;
	const float high1_m = decap_env_int("DECAP_HIGH1_MM", 300) / 1000.0f;
	if (!goals && decap_env_int("DECAP_RUN_CHASE", 1)) {
		// [game access: 2 lines]
			points_w = 0.0f;
		}
		// [game access]
			points_w *= 2.0f;
		}
	}

	float dm_gain_w = 1.0f;
	if (decap_env_int("DECAP_LEAD_DAMP", 0) && decap_root_value >= 0.0f) {
		float span = std::max(1, decap_env_int("DECAP_LEAD_DAMP_SPAN_MILLI", 200)) / 1000.0f;
		dm_gain_w = std::min(1.0f, std::max(0.0f, (1.0f - decap_root_value) / span));
		points_w *= dm_gain_w;
	}

	const bool rg_on = decap_rg_on();
	float rg_w = 0.0f, rg_end_w = 0.0f, rg_min = 0.0f;
	bool rg_sum = false;
	const float rg_cs = rg_on ? (float)decap_env_int("DECAP_RG_CS_PTS", 0) : 0.0f;
	if (rg_on) {
		rg_w = decap_rg_weight(me);
		rg_end_w = decap_env_int("DECAP_RG_END_MILLI", 700) / 1000.0f;
		rg_min = decap_env_int("DECAP_RG_MIN_MILLI", 50) / 1000.0f;
		rg_sum = decap_env_int("DECAP_RG_SUM", 0) != 0;
	}
	std::vector<float> score(nm, 0.0f);
	std::vector<char> safe(nm, 0);
	std::vector<double> bad(nm, 0.0);
	decap_diag.n = std::min(nm, (int)DECAP_DIAG_MAX);
	float chase_worst_w, chase_w;
	bool chasing = decap_chase_state(me, &chase_worst_w, &chase_w);
	std::vector<int> xp_rep;
	if (decap_xp.on) {
		decap_xp.calls++;
		for (int t = 0; t < nt; t++) xp_rep.push_back(decap_xp_reply(theirs[t], 0));
	}
	for (int c = 0; c < nm; c++) {
		float sum = 0.0f, worst = 10.0f;
		float vt = 0.0f, low = 10.0f, val = 0.0f;
		int why = 0, got = 0;
		int surv = 0, dq_frame = 1 << 30;
		int xp_slot = decap_xp.on ? decap_xp_slot(mine[c]) : -1;
		int nt_c = nt;
		if (mask) {
			nt_c = 0;
			for (int t = 0; t < nt; t++) nt_c += (*mask)[c * nt + t] != 0;
			if (nt_c == 0) bad[c] = std::max(bad[c], 500.0);
		}
		for (int t = 0; t < nt; t++) {
			if (mask && !(*mask)[c * nt + t]) continue;
			const decap_outcome &o = res[c * nt + t];
			if (!o.ok) {
				bad[c] = std::max(bad[c], 500.0);
				why = why ? why : 4;
				if (xp_slot >= 0) decap_xp_add(xp_slot, xp_rep[t], o, NAN, 0);
				continue;
			}
			got++;
			vt += o.v_toward1;
			low = std::min(low, o.low1);
			val += o.value;
			if (o.lost1) why = 1;
			else if (o.over1) {}
			else if (o.timer1 >= limit) why = why == 1 ? 1 : 2;
			else if (o.low1 < min_low) why = (why == 1 || why == 2) ? why : 3;
			if (o.near1 && !o.lost1 && !o.over1) {
				why = why ? why : 2;
				bad[c] = std::max(bad[c], 30.0 - o.margin1);
			}
			if (o.lost1) dq_frame = std::min(dq_frame, o.end_frame);
			else if (o.over1 || (o.timer1 < limit && o.low1 >= min_low && !o.near1)) surv++;
			if (o.lost1) {
				bad[c] = std::max(bad[c], 1000.0 + (start + frames - o.end_frame));
			}
			else if (o.over1) {
			}
			else if (o.timer1 >= limit) {
				bad[c] = std::max(bad[c], 10.0 + o.timer1);
			}
			else if (o.low1 < min_low) {
				bad[c] = std::max(bad[c], 1.0 + (min_low - o.low1) * 10.0);
			}
			float v = o.value + points_w * tanhf(urgency * o.points / 5000.0f) + (goals ? decap_goal_bonus(sit, o, goal_scale, arrive_w, arrive_g) : 0.0f);
			if (air1_w > 0.0f && !sit.near && o.air1 > air1_h) v -= air1_w;
			if (high1_w > 0.0f && !sit.near && o.high1 > high1_m) v -= high1_w * (o.high1 - high1_m);

			if (carry_w > 0.0f && o.carry1 && !o.over1) v -= carry_w * std::max(0.0f, o.value);
			if (dm_w > 0.0f) {
				if (dm_gain_w < 1.0f) v += dm_w * ((dm_gain_w * o.dm_op1 - o.dm_me1) + dm_cont * (dm_gain_w * o.dm_op_c - o.dm_me_c));
				else v += dm_w * ((o.dm_op1 - o.dm_me1) + dm_cont * (o.dm_op_c - o.dm_me_c));
			}
			if (desperate) {
				v += press_w * (o.risk_op - o.risk_me);
			}
			else if (defend) {
				v -= press_w * o.risk_me;
			}
			else {

				v += press_edge * o.risk_op - edge_w * o.risk_me;
			}
			if (chasing) {

				float cterm = chase_w * tanhf((sit.d0 - o.d1) / 0.75f);
				if (chase_hit > 0.0f && o.dealt1 >= chase_hit) cterm = std::max(cterm, 0.0f);
				if (sit.d0 >= chase_near) v += cterm;
			}
			if (rg_on) {

				float rg = (rg_sum ? o.rg1 + rg_end_w * o.rg_end : std::max(o.rg1, rg_end_w * o.rg_end)) - rg_min;
				if (rg > 0.0f) v -= rg_w * rg;
				else rg = 0.0f;
				if (!decap_shadow_jobs) decap_rg_note(mine[c], rg);
			}
			{
				bool uns = o.lost1 || (!o.over1 && (o.timer1 >= limit || o.low1 < min_low || o.near1));
				decap_full.pv[c * nt + t] = v;
				decap_full.pj[c * nt + t] = decap_opp_obj(o, limit, min_low, oj_loss, oj_unsafe, oj_dm, oj_dealt);
				if (rg_on && rg_cs > 0.0f && !o.lost1 && o.rg1 > rg_min) decap_full.pj[c * nt + t] += rg_cs * (o.rg1 - rg_min);
				decap_full.pd[c * nt + t] = o.taken1;
				decap_full.pu[c * nt + t] = (uint8_t)((uns ? 1 : 0) | (o.lost1 ? 2 : 0) | 4);
			}
			if (xp_slot >= 0) {
				uint8_t fl = 1 | (o.lost1 ? 2 : 0) | (o.timer1 >= limit ? 4 : 0) | (o.low1 < min_low ? 8 : 0) | (o.over1 ? 16 : 0) |
				             (o.lop1 ? 32 : 0) | (o.lme_plan ? 64 : 0) | (o.lop_plan ? 128 : 0);
				decap_xp_add(xp_slot, xp_rep[t], o, v, fl);
			}
			sum += v;
			worst = std::min(worst, v);
		}
		if (bad[c] == 0.0) {
			score[c] = (1.0f - chase_worst_w) * sum / nt_c + chase_worst_w * worst;
			safe[c] = 1;
			pick.safe++;
		}
		decap_full.mean_v[c] = bad[c] == 0.0 ? sum / nt_c : NAN;
		decap_full.min_v[c] = bad[c] == 0.0 ? worst : NAN;
		decap_full.why[c] = bad[c] == 0.0 ? 0 : (uint8_t)(why ? why : 4);
		decap_full.value[c] = got ? val / got : NAN;
		decap_full.mean_all[c] = got ? sum / got : NAN;
		decap_full.min_all[c] = got ? worst : NAN;
		decap_full.n_ans[c] = got;
		decap_full.surv[c] = surv;
		decap_full.dq_frame[c] = dq_frame;
		if (c < DECAP_DIAG_MAX) {
			decap_diag.mean_v[c] = bad[c] == 0.0 ? sum / nt_c : NAN;
			decap_diag.min_v[c] = bad[c] == 0.0 ? worst : NAN;
			decap_diag.why[c] = bad[c] == 0.0 ? 0 : (uint8_t)(why ? why : 4);
			decap_diag.score[c] = bad[c] == 0.0 ? score[c] : NAN;
			decap_diag.v_toward[c] = got ? vt / got : NAN;
			decap_diag.low[c] = low;
			decap_diag.value[c] = got ? val / got : NAN;
		}
	}
	decap_lf_call(mine, theirs, res, mask, limit, min_low);
	if (pick.safe == 0) {
		int least = 0;
		if (decap_env_int("DECAP_RESCUE", 0)) {

			for (int c = 1; c < nm; c++) {
				if (decap_rescue_better(c, least)) least = c;
			}
		}
		else {
			for (int c = 1; c < nm; c++) {
				if (bad[c] < bad[least]) {
					least = c;
				}
			}
		}
		pick.candidate = least;
		return pick;
	}
	if (decap_plan_outcomes) return pick;

	float tau = std::max(1, decap_env_int("DECAP_TAU_MILLI", 30)) / 1000.0f;
	double u = (double)rand() / ((double)RAND_MAX + 1.0);
	pick.candidate = std::max(0, bot::pickCandidate(score, safe, tau, u, &pick.best));
	pick.score = score[pick.candidate];
	pick.plain = safe[0] ? score[0] : -1.0f;
	return pick;
}

struct decap_crab_state {
	bool low;
	bool crab;
};

// [game access]
{
	// [REDACTED: this function mainly reads the game's state; 3 lines]
}

static decap_crab_state decap_crab(int p)
{
	// [REDACTED: this function mainly reads the game's state; 23 lines]
}

static void decap_crab_edits(const Move move, std::vector<Move> &out)
{
	for (int left_top = 0; left_top < 2; left_top++) {
		int top_hip = left_top ? 15 : 14, bottom_hip = left_top ? 14 : 15;
		int top_pec = left_top ? 7 : 4, bottom_pec = left_top ? 4 : 7;
		Move c = move;
		c.states[12] = c.states[13] = 2;
		c.states[bottom_hip] = 1;
		c.states[top_hip] = 2;
		c.states[top_pec] = 2;
		c.states[bottom_pec] = 1;
		c.states[5] = c.states[8] = 4;
		c.states[10] = c.states[11] = 2;
		out.push_back(c);
	}
}

static void decap_crab_launch_edits(const Move move, const uint8_t prev[N_JOINTS], std::vector<Move> &out)
{
	if (prev[10] != 2 && prev[11] != 2) {
		return;
	}
	Move a = move;
	a.states[10] = a.states[11] = 1;
	out.push_back(a);
	Move b = a;
	b.states[6] = b.states[9] = 1;
	out.push_back(b);
}

static bool decap_launch_ready(int p)
{
	// [REDACTED: this function mainly reads the game's state; 12 lines]
}

static void decap_top_launch_edits(const Move move, std::vector<Move> &out)
{
	Move push = move;
	push.states[5] = push.states[8] = 2;
	push.states[10] = push.states[11] = 1;
	push.states[6] = push.states[9] = 1;
	out.push_back(push);
	for (int kick_right = 0; kick_right < 2; kick_right++) {
		Move c = push;
		int kick_knee = kick_right ? 16 : 17, other_knee = kick_right ? 17 : 16;
		int kick_hip = kick_right ? 14 : 15, other_hip = kick_right ? 15 : 14;
		c.states[kick_knee] = 1;
		c.states[other_knee] = 2;
		c.states[kick_hip] = 1;
		c.states[other_hip] = 2;
		c.states[12] = c.states[13] = 1;
		out.push_back(c);
	}
}

static void decap_top_setup_edits(const Move move, std::vector<Move> &out)
{
	for (int right_extended = 0; right_extended < 2; right_extended++) {
		Move c = move;
		c.states[10] = c.states[11] = 2;
		c.states[6] = c.states[9] = 1;
		c.states[14] = c.states[15] = 2;
		c.states[12] = c.states[13] = 1;
		c.states[16] = right_extended ? 1 : 2;
		c.states[17] = right_extended ? 2 : 1;
		out.push_back(c);
	}
}

static void decap_kick_edits(const Move move, const uint8_t prev[N_JOINTS], std::vector<Move> &out)
{
	static const int legs[2][3] = { { 16, 18, 14 }, { 17, 19, 15 } };
	for (int l = 0; l < 2; l++) {
		int knee = legs[l][0], ankle = legs[l][1], hip = legs[l][2];
		if (move.states[knee] == 1 && prev[knee] == 2) {
			Move a = move;
			a.states[ankle] = 4;
			out.push_back(a);
			Move b = a;
			b.states[hip] = 4;
			out.push_back(b);
			Move c = move;
			c.states[knee] = 3;
			out.push_back(c);
		}
	}
}
#endif

#if BOT_BUILD
#include "decap_open.inc"
#endif

#if BOT_BUILD

static decap_ponder_slot *decap_ponder = nullptr;
static pid_t decap_ponder_pid = 0;
static int decap_ponder_player = -1;
static struct timespec decap_ponder_t0;

// [game access]
{
	// [REDACTED: this function mainly reads the game's state; 20 lines]
}

bool decap_ponder_enabled()
{
	// [game access]
}

bool decap_ponder_active()
{
	return decap_ponder_pid > 0;
}

void decap_ponder_stop()
{
	// [REDACTED: this function mainly reads the game's state; 6 lines]
}

static void decap_ponder_start(int seat, int level, int budget_ms)
{
	// [REDACTED: this function mainly reads the game's state; 67 lines]
}

bool decap_ponder_step(int seat, int level, bool opponent_committed, int turn_seconds)
{
	// [REDACTED: this function mainly reads the game's state; 37 lines]
}
#endif

#include "decap_race.inc"
static Move decap_choose_move(int seat, int level)
{
	if (getenv("DECAP_SEED")) {
		// [game access]
	}

	// [game access: 2 lines]
	}
	// [game access: 3 lines]
#if BOT_BUILD

	decap_lf_begin();
	decap_t1_begin();
	int skill = decap_env_int("DECAP_SKILL_MILLI", -1);
	// [game access]
		skill_level = std::min(1000, skill) / 1000.0f;
	}
#endif

	Position position = decap_position_for(seat, skill_level, n_frames, true);
	Move chosen;
#if BOT_BUILD
	// [game access]
		return decap_open_choose(seat, position, skill_level, n_frames);
	}
#endif
#if BOT_BUILD
	// [game access]
		struct timespec t0;
		clock_gettime(CLOCK_MONOTONIC, &t0);
		decap_t2g_state t2g;
		decap_t2g_begin(t2g, seat);
		decap_decision_scripts.clear();
		decap_rg_seen.clear();
		uint8_t prev[N_JOINTS];
		for (int j = 0; j < N_JOINTS; j++) {
			// [game access]
		}

		// [game access]
		std::vector<Move> drawn;
		int n_draws = std::max(0, decap_env_int("DECAP_LOOKAHEAD_DRAWS", 12));
		float gprobs[N_JOINTS][4];
		// [game access]
			for (int k = 0; k < n_draws; k++) {
				Move r = mine[0];
				for (int attempt = 0; attempt < 4; attempt++) {
					for (int j = 0; j < N_JOINTS; j++) {
						int best = 0;
						float bv = -1e30f;
						for (int s2 = 0; s2 < 4; s2++) {
							float u = (rand() + 1.0f) / ((float)RAND_MAX + 2.0f);
							float g = -logf(-logf(u));
							float v = logf(std::max(1e-6f, gprobs[j][s2])) + g;
							if (v > bv) { bv = v; best = s2; }
						}
						r.states[j] = (uint8_t)(best + 1);
					}
					bool dup = memcmp(r.states, mine[0].states, sizeof(r.states)) == 0;
					for (const Move &q : drawn) dup = dup || memcmp(q.states, r.states, sizeof(r.states)) == 0;
					if (!dup) break;
				}
				drawn.push_back(r);
			}
		}
		else {
			// [game access]
		}
		std::vector<uint8_t> tags(mine.size(), 0);
		mine.insert(mine.end(), drawn.begin(), drawn.end());
		tags.resize(mine.size(), 1);
		Move keep = mine[0];
		for (int j = 0; j < N_JOINTS; j++) {
			keep.states[j] = prev[j];
		}
		mine.push_back(keep);
		tags.resize(mine.size(), 2);
		size_t base = mine.size();
		for (size_t c = 0; c < base && c < 5; c++) {
			decap_kick_edits(mine[c], prev, mine);
		}
		tags.resize(mine.size(), 3);

		int knn = decap_env_int("DECAP_KNN", 8);
		std::vector<Move> similar;
		if (knn > 0) {
			// [game access]
			for (Move &r : similar) {
				memcpy(r.grip, mine[0].grip, sizeof(r.grip));
				mine.push_back(r);
			}
		}
		tags.resize(mine.size(), 4);
		{
			int cb = decap_env_int("DECAP_COMEBACK", 0);
			if (cb > 0) {
				// [game access]
					memcpy(r.grip, mine[0].grip, sizeof(r.grip));
					mine.push_back(r);
				}
			}
			tags.resize(mine.size(), 11);
		}
		{
			int eg = decap_env_int("DECAP_ENDGAME_TURNS", 0);
			// [game access: 2 lines]
			if (eg > 0 && left <= eg && lead >= (float)decap_env_int("DECAP_ENDGAME_LEAD", 1000)) {
				static const char *escapes[] = {
					// [game access: 3 lines]
				for (const char *e : escapes) {
					Move r = mine[0];
					for (int j = 0; j < N_JOINTS; j++) r.states[j] = (uint8_t)(e[j] - '0');
					mine.push_back(r);
				}
				tags.resize(mine.size(), 9);
				for (const decap_script &s : decap_jump_templates()) {
					Move r = mine[0];
					memcpy(r.states, s.first.states, sizeof(r.states));
					mine.push_back(r);
				}
				tags.resize(mine.size(), 10);
			}
		}
		decap_crab_state crab = decap_crab(seat);
		bool ready = decap_launch_ready(seat);
		if (decap_env_int("DECAP_LAUNCH_EDITS", 0) && ready) {
			for (size_t c = 0; c < base && c < 2; c++) {
				decap_top_launch_edits(mine[c], mine);
			}
			tags.resize(mine.size(), 5);
			if (!(prev[10] == 2 && prev[11] == 2)) {
				decap_top_setup_edits(mine[0], mine);
			}
			tags.resize(mine.size(), 6);
		}
		if (decap_env_int("DECAP_CRAB_EDITS", 0)) {
			for (size_t c = 0; c < base && c < 2; c++) {
				if (crab.low) {
					decap_crab_edits(mine[c], mine);
				}
				if (crab.crab) {
					decap_crab_launch_edits(mine[c], prev, mine);
				}
			}
		}
		tags.resize(mine.size(), 7);
		std::vector<Move> unique;
		std::vector<uint8_t> utags;
		for (size_t c = 0; c < mine.size(); c++) {
			bool seen = false;
			for (size_t u = 0; u < unique.size() && !seen; u++) {
				seen = memcmp(unique[u].states, mine[c].states, sizeof(mine[c].states)) == 0;
			}
			if (!seen) {
				unique.push_back(mine[c]);
				utags.push_back(tags[c]);
			}
		}
		mine.swap(unique);
		memset(&decap_diag, 0, sizeof(decap_diag));
		for (size_t c = 0; c < utags.size() && c < DECAP_DIAG_MAX; c++) decap_diag.tag[c] = utags[c];
		decap_diag.ready = ready;
		{
			float u[2];
			decap_diag.d0 = decap_chest_dist(seat, 1 - seat, u);
		}

		Position their_pos = decap_position_for(1 - seat, skill_level, n_frames, true);
		std::vector<Move> theirs(1, keep);
		for (int j = 0; j < N_JOINTS; j++) {
			// [game access]
		}
		decap_xp_reset();
		decap_xp_label(theirs, 0, 1);
		// [game access: 2 lines]
		theirs.insert(theirs.end(), replies.begin(), replies.end());
		decap_xp_label(theirs, 1, 2);
		int knn_replies = decap_env_int("DECAP_KNN_REPLIES", 0);
		if (knn_replies > 0) {
			// [game access]
			size_t k0 = theirs.size();
			theirs.insert(theirs.end(), similar.begin(), similar.end());
			decap_xp_label(theirs, k0, 3);
		}
		const bool rr_snap = decap_rr_begin(mine);
		bool race_dec = decap_race_hook(seat, mine, theirs, their_pos, n_frames, skill_level, t0, decap_diag.d0);
		// [game access: 2 lines]
		decap_pick pick = decap_lookahead(seat, mine, theirs, n_frames, skill_level, plan_seed);

		// [game access]
			decap_rr_restore(mine, pick, t0, decap_diag.d0);
			race_dec = false;
			// [game access: 2 lines]
			pick = decap_lookahead(seat, mine, theirs, n_frames, skill_level, plan_seed);
		}
		const decap_full_t round1 = decap_full;
		const bool full_res = decap_env_int("DECAP_FULL_RESULTS", 0) != 0;
		chosen = mine[pick.candidate];
		decap_diag.played = pick.candidate;
		decap_publish(chosen, pick.score, 1);
		{
			decap_last_dec.req = position;
			decap_last_dec.cands.clear();
			for (size_t c = 0; c < mine.size() && c < DECAP_DIAG_MAX; c++) {
				decap_dec_cand d;
				memcpy(d.joints, mine[c].states, N_JOINTS);
				d.grip[0] = mine[c].grip[0];
				d.grip[1] = mine[c].grip[1];
				d.safe = decap_diag.why[c] == 0;
				d.tag = decap_diag.tag[c];
				d.mean_v = decap_diag.mean_v[c];
				d.min_v = decap_diag.min_v[c];
				d.fscore = NAN;
				decap_last_dec.cands.push_back(d);
			}
			decap_last_dec.played = chosen;
			decap_last_dec.played_idx = pick.candidate;
			decap_last_dec.cem_note = 0;
			decap_last_dec.ready = decap_diag.ready;
			decap_last_dec.d0 = decap_diag.d0;
			decap_last_dec.score = pick.score;
			decap_last_dec.valid = true;
		}

		int refine = decap_env_int("DECAP_REFINE", 0), refined = 0;
		if (refine > 0 && pick.safe > 0 && pick.results > 0) {
			int parents = decap_env_int("DECAP_REFINE_PARENTS", 4);
			std::vector<std::pair<float, int>> ranked;
			for (int c = 0; c < (int)mine.size() && c < DECAP_DIAG_MAX; c++) {
				if (decap_diag.why[c] == 0 && std::isfinite(decap_diag.score[c])) {
					ranked.push_back(std::make_pair(decap_diag.score[c], c));
				}
			}
			std::sort(ranked.begin(), ranked.end(), std::greater<std::pair<float, int>>());
			std::vector<Move> muts;
			for (int k = 0; k < (int)ranked.size() && k < parents; k++) {
				const Move &par = mine[ranked[k].second];
				for (int m = 0; m < refine; m++) {
					Move r = par;
					int changes = 1 + rand() % 3;
					for (int x = 0; x < changes; x++) {
						int j = rand() % N_JOINTS;
						// [game access]
						int st = 1 + rand() % 4;
						if (st == r.states[j]) st = 1 + st % 4;
						r.states[j] = (uint8_t)st;
					}
					bool seen = false;
					for (const Move &q : mine) seen |= memcmp(q.states, r.states, sizeof(r.states)) == 0;
					for (const Move &q : muts) seen |= memcmp(q.states, r.states, sizeof(r.states)) == 0;
					if (!seen) muts.push_back(r);
				}
			}
			if (!muts.empty()) {
				decap_diag_t first = decap_diag;
				decap_pick pick2 = decap_lookahead(seat, muts, theirs, n_frames, skill_level, plan_seed);
				if (pick2.safe > 0 && pick2.best > pick.best) {
					chosen = muts[pick2.candidate];
					pick.best = pick2.best;
					pick.score = pick2.score;
					refined = 1;
				}
				decap_diag = first;
				decap_diag.played = refined ? -1 : pick.candidate;
			}
		}

		// [game access]
		const bool rescue_cem = pick.safe == 0 && pick.results > 0 && cem > 0 && decap_env_int("DECAP_RESCUE", 0) != 0;
		// [game access]
			long budget_ms = decap_env_int("DECAP_LOOKAHEAD_MS", 3000);
			int plan_budget = decap_env_int("DECAP_PLAN_BUDGET", 300), plans = (int)(mine.size() * theirs.size());
			float tau = std::max(1, decap_env_int("DECAP_TAU_MILLI", 30)) / 1000.0f;
			float anchor = decap_env_int("DECAP_CEM_ANCHOR_MILLI", 10) / 1000.0f;
			decap_diag_t first = decap_diag;
			std::vector<Move> cand(mine);

			// [game access]
			const long ko_full_ms = budget_ms;
			bool ko_changed = false;
			if (ko_on) budget_ms = std::max(1L, budget_ms * std::min(100, std::max(5, decap_env_int("DECAP_KO_MAIN_PCT", 45))) / 100);
			std::vector<float> c_mean, c_min;
			std::vector<char> c_safe;

			auto pv_row = [](const decap_full_t &f, size_t c) {
				std::vector<float> r;
				if (f.nt > 0 && (c + 1) * (size_t)f.nt <= f.pv.size()) r.assign(f.pv.begin() + c * f.nt, f.pv.begin() + (c + 1) * f.nt);
				return r;
			};
			std::vector<std::vector<float>> c_pv;
			for (size_t c = 0; c < mine.size(); c++) c_pv.push_back(pv_row(round1, c));
			for (size_t c = 0; c < mine.size(); c++) {
				if (rescue_cem) {
					// [game access]
					c_safe.push_back(r > -9.0f);
					c_mean.push_back(r);
					c_min.push_back(r);
					continue;
				}
				if (full_res) {
					bool ok = round1.ok(c);
					c_safe.push_back(ok);
					c_mean.push_back(ok ? round1.mean_v[c] : -9.0f);
					c_min.push_back(ok ? round1.min_v[c] : -9.0f);
					continue;
				}
				bool ok = c < DECAP_DIAG_MAX && first.why[c] == 0 && std::isfinite(first.mean_v[c]);
				c_safe.push_back(ok);
				c_mean.push_back(ok ? first.mean_v[c] : -9.0f);
				c_min.push_back(ok ? first.min_v[c] : -9.0f);
			}
			float xw_worst, xw_chase;
			decap_chase_state(seat, &xw_worst, &xw_chase);
			auto stage_score = [&](size_t c) { return (1.0f - xw_worst) * c_mean[c] + xw_worst * c_min[c]; };
			float probs[N_JOINTS][4];
			// [game access]
			auto logprob = [&](const Move &m) {
				if (!have_probs) return 0.0f;
				float s = 0.0f;
				for (int j = 0; j < N_JOINTS; j++) {
					int st = m.states[j];
					s += logf(std::max(1e-6f, st >= 1 && st <= 4 ? probs[j][st - 1] : 1e-6f));
				}
				return s / N_JOINTS;
			};
			auto seen = [&](const Move &r) {
				for (const Move &q : cand) {
					if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return true;
				}
				return false;
			};

			long lp_ms = 0;
			size_t lp_c0 = 0, lp_cn = 0;
			std::vector<float> lp_kj;
			bool lp_bonus_ok = true;
			const char *sh_gate = "";
			decap_cl_state cl;
			if (decap_env_int("DECAP_LP", 0) && !rescue_cem) {
				// [game access: 2 lines]
				const int lp_eg = decap_env_int("DECAP_ENDGAME_TURNS", 0);
				const bool lp_escape = lp_eg > 0 && lp_left <= lp_eg && lp_lead >= (float)decap_env_int("DECAP_ENDGAME_LEAD", 1000);
				const long lp_start = decap_ms_since(t0);

				const long lp_cap = std::min((long)decap_env_int("DECAP_LP_MS", 3500), budget_ms * 4 / 10 - lp_start);
				const bool lp_shadow = decap_env_int("DECAP_SHADOW", 0) != 0;
				if (first.d0 >= decap_env_int("DECAP_LP_FROM_MILLI", 2200) / 1000.0f && lp_lead <= (float)decap_env_int("DECAP_LP_MAX_LEAD", 0) &&
				    !lp_escape && lp_cap > 0 && !lp_shadow) {
					const int lp_pop = std::min(512, std::max(2, decap_env_int("DECAP_LP_POP", 64)));
					const int lp_gens = std::max(1, decap_env_int("DECAP_LP_GENS", 4));
					const int lp_elites = std::min(lp_pop, std::max(1, decap_env_int("DECAP_LP_ELITES", 8)));
					const int lp_keep = std::min(16, std::max(0, decap_env_int("DECAP_LP_KEEP", 4)));
					const float lp_pts = decap_env_int("DECAP_LP_PTS_MILLI", 0) / 1000.0f;

					const int lp_opp_keep = decap_env_int("DECAP_LP_OPP_KEEP", 0) != 0;
					struct lp_seq { Move m[3]; float j, d1, d2, d3; };
					const Move lp_seed = mine[0];
					float lp_p[3][N_JOINTS][4];
					for (int t = 0; t < 3; t++) {
						for (int j = 0; j < N_JOINTS; j++) {
							for (int s2 = 0; s2 < 4; s2++) lp_p[t][j][s2] = lp_seed.states[j] == s2 + 1 ? 0.4f : 0.2f;
						}
					}

					unsigned lp_rs = (plan_seed ^ 0x6c703235u) | 1u;
					auto lp_rand = [&]() {
						lp_rs ^= lp_rs << 13;
						lp_rs ^= lp_rs >> 17;
						lp_rs ^= lp_rs << 5;
						return (float)(lp_rs >> 8) / 16777216.0f;
					};
					const std::vector<Move> lp_reply(1, theirs[0]);
					std::vector<lp_seq> lp_all;
					lp_seq lp_best = {};
					bool lp_have = false;
					int lp_done = 0, lp_sims = 0;
					long lp_gen_ms = 0;
					const bool lp_xon = decap_xp.on;
					decap_xp.on = false;
					for (int g = 0; g < lp_gens; g++) {

						const long lp_at = decap_ms_since(t0) - lp_start;
						const long hold = std::max(lp_cap * 15 / 100, 2 * lp_gen_ms * lp_keep * (long)theirs.size() / lp_pop);
						long left = lp_cap - hold - lp_at;
						if (left <= 0) break;
						std::vector<lp_seq> seqs(lp_pop);
						std::vector<Move> firsts(lp_pop);
						std::vector<std::vector<Move>> scripts(lp_pop);
						for (int i = 0; i < lp_pop; i++) {
							lp_seq &q = seqs[i];
							q.j = -1000.0f;
							q.d1 = q.d2 = q.d3 = NAN;
							for (int t = 0; t < 3; t++) {
								q.m[t] = lp_seed;
								if (g == 0 && i == 0) continue;
								for (int j = 0; j < N_JOINTS; j++) {
									// [game access]
									float r = lp_rand() * (lp_p[t][j][0] + lp_p[t][j][1] + lp_p[t][j][2] + lp_p[t][j][3]);
									int s2 = 0;
									while (s2 < 3 && (r -= lp_p[t][j][s2]) >= 0.0f) s2++;
									q.m[t].states[j] = (uint8_t)(s2 + 1);
								}
							}
							firsts[i] = q.m[0];
							scripts[i].assign(q.m + 1, q.m + 3);
						}
						std::vector<decap_outcome> outs;
						decap_plan_scripts = &scripts;
						decap_plan_outcomes = &outs;
						decap_plan_opp_keep = lp_opp_keep;
						decap_plan_budget_ms = std::max(1L, left);

						decap_pick lp_pick = decap_lookahead(seat, firsts, lp_reply, n_frames, skill_level, plan_seed);
						decap_plan_budget_ms = -1;
						decap_plan_opp_keep = 0;
						decap_plan_outcomes = nullptr;
						decap_plan_scripts = nullptr;
						lp_sims += lp_pick.results;
						lp_done++;
						lp_gen_ms = decap_ms_since(t0) - lp_start - lp_at;
						std::vector<std::pair<float, int>> rank;
						for (int i = 0; i < lp_pop && i < (int)outs.size() && i < (int)decap_full.pu.size(); i++) {
							const decap_outcome &o = outs[i];
							const uint8_t pu = decap_full.pu[i];
							if (!o.ok || !(pu & 4) || (pu & 1) || o.lost1 || o.lme_plan) continue;

							if (!std::isfinite(o.d1)) continue;
							lp_seq &q = seqs[i];
							q.d1 = o.d1;
							q.d2 = std::isfinite(o.d2) ? o.d2 : o.d1;
							q.d3 = std::isfinite(o.d3) ? o.d3 : q.d2;
							q.j = -(0.4f * q.d2 + q.d3) + lp_pts * (o.dealt - o.taken) / 5000.0f;
							if (!std::isfinite(q.j)) {
								q.j = -1000.0f;
								continue;
							}
							lp_all.push_back(q);
							rank.push_back(std::make_pair(q.j, i));
						}
						if (lp_have) rank.push_back(std::make_pair(lp_best.j, -1));
						std::sort(rank.begin(), rank.end(), std::greater<std::pair<float, int>>());
						const int ne = std::min((int)rank.size(), lp_elites);
						if (ne > 0) {
							float freq[3][N_JOINTS][4];
							memset(freq, 0, sizeof(freq));
							for (int e = 0; e < ne; e++) {
								const lp_seq &q = rank[e].second < 0 ? lp_best : seqs[rank[e].second];
								for (int t = 0; t < 3; t++) {
									for (int j = 0; j < N_JOINTS; j++) {
										int st = q.m[t].states[j];
										if (st >= 1 && st <= 4) freq[t][j][st - 1] += 1.0f / ne;
									}
								}
							}
							for (int t = 0; t < 3; t++) {
								for (int j = 0; j < N_JOINTS; j++) {
									float tot = 0.0f;
									for (int s2 = 0; s2 < 4; s2++) {
										lp_p[t][j][s2] = std::max(0.03f, 0.3f * lp_p[t][j][s2] + 0.7f * freq[t][j][s2]);
										tot += lp_p[t][j][s2];
									}
									for (int s2 = 0; s2 < 4; s2++) lp_p[t][j][s2] /= tot;
								}
							}
							if (rank[0].second >= 0) {
								lp_best = seqs[rank[0].second];
								lp_have = true;
							}
						}
					}
					decap_xp.on = lp_xon;

					std::stable_sort(lp_all.begin(), lp_all.end(), [](const lp_seq &a, const lp_seq &b) { return a.j > b.j; });
					std::vector<Move> lp_first;
					for (const lp_seq &q : lp_all) {
						if ((int)lp_first.size() >= lp_keep) break;
						if (seen(q.m[0]) || decap_script_for(q.m[0]) != nullptr) continue;
						decap_script s;
						s.first = q.m[0];
						s.rest.assign(q.m + 1, q.m + 3);
						decap_decision_scripts.push_back(s);
						lp_first.push_back(q.m[0]);
						lp_kj.push_back(q.j);
					}
					float lp_d3[3] = { NAN, NAN, NAN };
					if (!lp_first.empty()) {
						std::vector<decap_outcome> outs;
						decap_plan_outcomes = &outs;
						decap_plan_budget_ms = std::max(1L, lp_cap - (decap_ms_since(t0) - lp_start));

						const decap_pick lp_kp = decap_lookahead(seat, lp_first, theirs, n_frames, skill_level, plan_seed);
						decap_plan_budget_ms = -1;
						decap_plan_outcomes = nullptr;
						for (size_t t = 0; t < theirs.size() && t < outs.size(); t++) {
							const decap_outcome &o = outs[t];
							if (!o.ok) continue;
							float d3 = std::isfinite(o.d3) ? o.d3 : (std::isfinite(o.d2) ? o.d2 : o.d1);
							if (t == 0) lp_d3[0] = d3;
							lp_d3[1] = std::isfinite(lp_d3[1]) ? std::min(lp_d3[1], d3) : d3;
							lp_d3[2] = std::isfinite(lp_d3[2]) ? std::max(lp_d3[2], d3) : d3;
						}
						lp_c0 = cand.size();
						lp_cn = lp_first.size();
						for (size_t c = 0; c < lp_first.size(); c++) c_pv.push_back(pv_row(decap_full, c));
						for (size_t c = 0; c < lp_first.size(); c++) {
							bool ok = lp_kp.results > 0 &&
							          (full_res ? decap_full.ok(c) : (c < DECAP_DIAG_MAX && decap_diag.why[c] == 0 && std::isfinite(decap_diag.mean_v[c])));
							cand.push_back(lp_first[c]);
							c_safe.push_back(ok);
							c_mean.push_back(ok ? (full_res ? decap_full.mean_v[c] : decap_diag.mean_v[c]) : -9.0f);
							c_min.push_back(ok ? (full_res ? decap_full.min_v[c] : decap_diag.min_v[c]) : -9.0f);
						}
						cem_note |= 64;
					}
					lp_ms = std::max(1L, decap_ms_since(t0) - lp_start);

					const bool lp_log = decap_env_int("DECAP_LOOKAHEAD_LOG", 0) != 0, lp_file = decap_env_int("DECAP_LP_LOG", 0) != 0;
					if (lp_log || lp_file) {
						char line[400];
						// [game access: 4 lines]
						const char *dir = getenv("DECAP_DECISION_LOG");
						if (lp_file && dir && *dir) {
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
				}

				if (lp_shadow && first.d0 >= decap_env_int("DECAP_LP_FROM_MILLI", 2200) / 1000.0f && lp_lead <= (float)decap_env_int("DECAP_LP_MAX_LEAD", 0) &&
				    !lp_escape && lp_cap > 0) {

					if (decap_env_int("DECAP_CL", 0) == 1) {
						const long cl_t = decap_ms_since(t0);
						cl.ran = true;
						cl.at_ms = cl_t;
						cl.model = decap_env_int("DECAP_CL_MODEL", 0) == 1 ? 1 : 0;
						cl.threads = std::max(1, std::min(64, decap_env_int("DECAP_CL_THREADS", 8)));
						cl.budget = std::max(1L, std::min((long)decap_env_int("DECAP_CL_MS", 4000), budget_ms * 3 / 10));
						// [game access]
						auto dig = [](const Move &r) { std::string s(N_JOINTS, '0'); for (int j = 0; j < N_JOINTS; j++) s[j] = (char)('0' + r.states[j]); return s; };
						std::string cs1 = dig(keep), cs2;
						for (size_t c = 0; c < cand.size(); c++) { cs1 += dig(cand[c]); cs2 += dig(cand[c]); }
						cl.n_seed1 = (int)(cs1.size() / N_JOINTS);
						cl.n_seed2 = (int)(cs2.size() / N_JOINTS);
						// [game access: 2 lines]
						decap_cl_req rq;
						memset(&rq, 0, sizeof(rq));
						rq.H = cl_h.get();
						rq.me = seat;
						// [game access: 2 lines]
						rq.model = cl.model;
						rq.seed = cl.seed;
						rq.n1 = std::max(1, decap_env_int("DECAP_CL_N1", 100));
						rq.n2 = std::max(1, decap_env_int("DECAP_CL_N2", 4));
						rq.gens = std::max(1, decap_env_int("DECAP_CL_GENS", 60));
						rq.threads = cl.threads;
						rq.budget_ms = cl.budget;
						rq.seeds1 = cs1.c_str();
						rq.n_seeds1 = cl.n_seed1;
						rq.seeds2 = cs2.c_str();
						rq.n_seeds2 = cl.n_seed2;
						rq.topk = std::max(1, std::min((int)DECAP_CL_TOPK, decap_env_int("DECAP_CL_K", 8)));
						rq.obj = decap_env_int("DECAP_CL_OBJ", 0) == 1 ? 1 : 0;
						// [game access]
						decap_cl_search(rq, cl.res);
						cl.ms = std::max(1L, decap_ms_since(t0) - cl_t);
						decap_cl_dump(cl, cs1, cs2, seat);
					}
					const int sh_pop = std::min(512, std::max(2, decap_env_int("DECAP_LP_POP", 64)));
					const int sh_gens = std::max(1, decap_env_int("DECAP_LP_GENS", 4));
					const int sh_elites = std::min(sh_pop, std::max(1, decap_env_int("DECAP_LP_ELITES", 8)));
					const int sh_keep = std::min(16, std::max(0, decap_env_int("DECAP_LP_KEEP", 4)));
					const int sh_m = std::min(512, std::max(4, decap_env_int("DECAP_SHADOW_OPP", 128)));
					const int sh_top = std::max(1, decap_env_int("DECAP_SHADOW_TOP", 16));
					const int sh_freeze = decap_env_int("DECAP_SHADOW_FREEZE", 1) != 0;
					const int sh_verify = decap_env_int("DECAP_SHADOW_VERIFY", 0) != 0;
					const char *sh_dump = getenv("DECAP_SHADOW_DUMP");
					const bool sh_dumping = sh_dump && *sh_dump;
					// [game access]
					sh_par.gamma = decap_env_int("DECAP_SHADOW_GAMMA_MILLI", 900) / 1000.0;
					sh_par.wc2 = decap_env_int("DECAP_SHADOW_WC2_MILLI", 50) / 1000.0;
					sh_par.wc3 = decap_env_int("DECAP_SHADOW_WC3_MILLI", 120) / 1000.0;
					sh_par.rw = lp_lead <= -(float)decap_env_int("DECAP_SHADOW_BEHIND", 3000) ? decap_env_int("DECAP_SHADOW_RISK_BEHIND_MILLI", 500) / 1000.0
					                                                                          : decap_env_int("DECAP_SHADOW_RISK_MILLI", 1000) / 1000.0;
					sh_par.pen2 = decap_env_int("DECAP_SHADOW_PEN2_MILLI", 100) / 1000.0;
					sh_par.pen3 = decap_env_int("DECAP_SHADOW_PEN3_MILLI", 30) / 1000.0;
					sh_par.margin = decap_env_int("DECAP_SHADOW_MARGIN_MM", 10) / 1000.0;
					sh_par.d0 = (double)first.d0;
					// [game access]
					sh_par.air_w = decap_env_int("DECAP_SHADOW_AIR_MILLI", 0) / 1000.0;
					sh_par.air_h = decap_env_int("DECAP_SHADOW_AIR_MM", 50) / 1000.0;
					sh_par.high_w = decap_env_int("DECAP_SHADOW_HIGH_MILLI", 0) / 1000.0;
					sh_par.high_m = decap_env_int("DECAP_SHADOW_HIGH_MM", 300) / 1000.0;
					sh_par.apex = decap_env_int("DECAP_HIGH_APEX", 0) == 1;
					sh_par.high_all = decap_env_int("DECAP_SHADOW_HIGH_ALL", 0) == 1;
					if (sh_par.apex) {
						// [game access: 8 lines]
						}
					}
					// [game access]
					const size_t sh_slot_meta = sizeof(decap_shadow_meta), sh_slot_cap = sh_slot_meta + sh_capn * sizeof(double);
					unsigned sh_rs = (plan_seed ^ 0x73683238u) | 1u;
					auto sh_rand = [&]() {
						sh_rs ^= sh_rs << 13;
						sh_rs ^= sh_rs >> 17;
						sh_rs ^= sh_rs << 5;
						return sh_rs >> 8;
					};
					auto sh_mutate = [&](const Move &base, int p) {
						Move r = base;
						int free_j[N_JOINTS], nf = 0;
						// [game access]
						int k = std::min(nf, 1 + (int)(sh_rand() % 6));
						for (int x = 0; x < k; x++) {
							int y = x + (int)(sh_rand() % (unsigned)(nf - x));
							std::swap(free_j[x], free_j[y]);
							int j = free_j[x];
							int st = 1 + (int)(sh_rand() % 3);
							if (st >= r.states[j]) st++;
							r.states[j] = (uint8_t)st;
						}
						return r;
					};
					auto same = [](const Move &a, const Move &b) { return memcmp(a.states, b.states, sizeof(a.states)) == 0; };
					const std::vector<Move> sh_reply(1, theirs[0]);
					std::vector<char *> sh_maps;
					std::vector<size_t> sh_mapn;

					auto sh_run = [&](const std::vector<decap_shadow_job> &jobs, size_t slot, int freeze, long budget, std::vector<decap_outcome> &outs) -> char * {
						const size_t n = jobs.size();
						if (n == 0) return nullptr;
						// [game access: 2 lines]
						sh_maps.push_back((char *)m);
						sh_mapn.push_back(n * slot);
						std::vector<Move> firsts(n);
						for (size_t i = 0; i < n; i++) firsts[i] = jobs[i].bot[0];
						decap_shadow_jobs = &jobs;
						decap_shadow_shm = (char *)m;
						decap_shadow_slot = slot;
						decap_shadow_freeze = freeze;
						decap_plan_outcomes = &outs;
						decap_plan_budget_ms = std::max(1L, budget);
						decap_lookahead(seat, firsts, sh_reply, n_frames, skill_level, plan_seed);
						decap_plan_budget_ms = -1;
						decap_plan_outcomes = nullptr;
						decap_shadow_freeze = 0;
						decap_shadow_jobs = nullptr;
						decap_shadow_shm = nullptr;
						decap_shadow_slot = 0;
						return (char *)m;
					};
					auto sh_meta = [&](char *m, size_t slot, size_t i) { return (const decap_shadow_meta *)(m + i * slot); };
					auto sh_answered = [&](const std::vector<decap_outcome> &outs, char *m, size_t slot, size_t i) {
						return m && i < outs.size() && outs[i].ok == 1 && sh_meta(m, slot, i)->ok == 1;
					};

					auto sh_turn_ok = [&](const decap_shadow_meta *mt, int t, bool bot) {
						const int l = bot ? mt->lme[t] : mt->lop[t];
						// [game access]
						const bool tend = mt->ran[t] && mt->over[t] && !mt->lme[t] && !mt->lop[t];
						// [game access]
					};
					auto sh_known_survives = [&](const decap_shadow_meta *mt, bool bot) {
						for (int t = 0; t < 3; t++) {
							if (!mt->ran[t]) return t > 0 && mt->over[t - 1] && !mt->lme[t - 1] && !mt->lop[t - 1];
							if (!sh_turn_ok(mt, t, bot)) return false;
						}
						return true;
					};
					const long sh_start = decap_ms_since(t0);
					auto sh_left = [&](long hold) { return lp_cap - hold - (decap_ms_since(t0) - sh_start); };
					const bool sh_xon = decap_xp.on;
					decap_xp.on = false;
					char line[1200];
					int sh_np = 0, sh_bpk = -1, sh_opk = -1, sh_nopp = 0, sh_valid[3] = { 0, 0, 0 }, sh_done = 0, sh_seqs = 0, sh_scored = 0;
					long sh_ms_part = 0, sh_ms_opp = 0, sh_ms_opp_free = -1, sh_ms_g1 = 0, sh_ms_g1_free = -1;
					int sh_vcases = 0, sh_vmis = 0, sh_vtrunc = 0;
					const char *sh_abort = nullptr;
					struct sh_seq { Move m[3]; double J, V, reach, close, neg, touch[3], predD[3]; int fl, gen, clx; };
					std::vector<sh_seq> sh_all;
					std::vector<decap_shadow_job> sh_ojobs, sh_g1jobs;
					std::vector<decap_outcome> sh_oouts, sh_g1outs;
					char *sh_omap = nullptr, *sh_g1map = nullptr;
					size_t sh_g1slot = 0;
					std::vector<uint8_t> sh_g1pu;
					std::vector<int> sh_okeep;
					// [game access]
					Move sh_seed = chosen;
					decap_shadow_job sh_bp, sh_op;
					for (int t = 0; t < 3; t++) sh_bp.bot[t] = sh_bp.opp[t] = sh_op.bot[t] = sh_op.opp[t] = theirs[0];
					for (int t = 0; t < 3; t++) sh_bp.bot[t] = chosen;
					// [game access]
					else if (sh_left(lp_cap * 15 / 100) <= 0) sh_abort = "no time left";

					if (!sh_abort) {
						std::vector<Move> bc, oc;
						auto addu = [&](std::vector<Move> &v, const Move &r) {
							for (const Move &x : v) if (same(x, r)) return;
							v.push_back(r);
						};
						addu(bc, chosen);
						addu(bc, mine[0]);
						{
							Move kp = chosen;
							for (int j = 0; j < N_JOINTS; j++) kp.states[j] = keep.states[j];
							addu(bc, kp);
						}
						std::vector<std::pair<float, int>> r1;
						for (size_t c = 0; c < mine.size(); c++) if (c_safe[c]) r1.push_back(std::make_pair(stage_score(c), (int)c));
						std::stable_sort(r1.begin(), r1.end(), [](const std::pair<float, int> &a, const std::pair<float, int> &b) { return a.first > b.first; });
						for (size_t k = 0; k < r1.size() && bc.size() < 8; k++) addu(bc, mine[r1[k].second]);
						for (size_t t = 0; t < theirs.size() && oc.size() < 8; t++) addu(oc, theirs[t]);
						for (int k = 0; k < 64 && oc.size() < 8; k++) addu(oc, sh_mutate(theirs[0], 1 - seat));
						const bool partners = !sh_freeze || sh_verify;
						std::vector<decap_shadow_job> pj(partners ? std::max(bc.size(), oc.size()) : bc.size());
						for (size_t k = 0; k < pj.size(); k++) {
							for (int t = 0; t < 3; t++) {
								pj[k].bot[t] = bc[std::min(k, bc.size() - 1)];
								pj[k].opp[t] = oc[partners ? std::min(k, oc.size() - 1) : 0];
							}
							pj[k].subj = partners ? -1 : 0;
							pj[k].score = 0;
							pj[k].keep_cap = 0;
							pj[k].keep_pairs = 0;
						}
						std::vector<decap_outcome> po;
						const long tp = decap_ms_since(t0);
						char *pm = sh_run(pj, sh_slot_meta, partners ? 0 : 1, sh_left(lp_cap * 15 / 100), po);
						sh_ms_part = decap_ms_since(t0) - tp;
						sh_np = (int)pj.size();
						int seed_k = -1;
						for (size_t k = 0; k < pj.size() && k < bc.size(); k++) {
							if (!sh_answered(po, pm, sh_slot_meta, k)) continue;
							const decap_shadow_meta *mt = sh_meta(pm, sh_slot_meta, k);
							if (seed_k < 0 && sh_turn_ok(mt, 0, true)) seed_k = (int)k;
							if (sh_bpk < 0 && sh_known_survives(mt, true)) sh_bpk = (int)k;
						}
						for (size_t k = 0; partners && k < pj.size() && k < oc.size(); k++) {
							if (!sh_answered(po, pm, sh_slot_meta, k)) continue;
							if (sh_opk < 0 && sh_known_survives(sh_meta(pm, sh_slot_meta, k), false)) sh_opk = (int)k;
						}
						if (seed_k >= 0) sh_seed = bc[seed_k];
						if (sh_bpk >= 0) for (int t = 0; t < 3; t++) sh_bp.bot[t] = bc[sh_bpk];
						if (sh_opk >= 0) for (int t = 0; t < 3; t++) sh_op.opp[t] = oc[sh_opk];
						// [game access: 2 lines]
					}

					// [game access]
					if (!sh_abort) {
						std::vector<Move> first1;
						std::vector<decap_shadow_job> &oj = sh_ojobs;
						auto addf = [&](const Move &r) {
							for (const Move &x : first1) if (same(x, r)) return false;
							first1.push_back(r);
							return true;
						};
						for (size_t t = 0; t < theirs.size() && (int)first1.size() < sh_m; t++) addf(theirs[t]);
						const size_t nbase = first1.size();
						for (int tries = 0; (int)first1.size() < sh_m && tries < 50 * sh_m; tries++) {
							addf(sh_mutate(first1[(size_t)tries % nbase], 1 - seat));
						}
						oj.resize(first1.size());
						for (size_t k = 0; k < first1.size(); k++) {
							decap_shadow_job &j = oj[k];
							j.opp[0] = first1[k];
							if (k % 3 == 0) j.opp[1] = j.opp[2] = first1[k];
							else {
								j.opp[1] = sh_mutate(first1[k], 1 - seat);
								j.opp[2] = sh_mutate(j.opp[1], 1 - seat);
							}
							for (int t = 0; t < 3; t++) j.bot[t] = sh_bp.bot[t];
							j.subj = 1;
							j.score = 0;
							j.keep_cap = 1;
							j.keep_pairs = 0;
						}
						const long to = decap_ms_since(t0);
						sh_omap = sh_run(oj, sh_slot_cap, sh_freeze, sh_left(lp_cap * 15 / 100), sh_oouts);
						sh_ms_opp = decap_ms_since(t0) - to;
						for (size_t k = 0; k < oj.size(); k++) {
							if (!sh_answered(sh_oouts, sh_omap, sh_slot_cap, k)) continue;
							const decap_shadow_meta *mt = sh_meta(sh_omap, sh_slot_cap, k);
							bool v[3];
							v[0] = sh_turn_ok(mt, 0, false);
							if (!v[0]) continue;
							for (int t = 1; t < 3; t++) v[t] = v[t - 1] && !mt->over[t - 1] && sh_turn_ok(mt, t, false);
							// [game access]
							const double *cap = (const double *)(sh_omap + k * sh_slot_cap + sh_slot_meta);
							for (int t = 0; t < 3; t++) {
								o.valid[t] = v[t];
								// [game access]
								sh_valid[t] += v[t];
							}
							sh_opps.push_back(o);
							sh_okeep.push_back((int)k);
						}
						sh_nopp = (int)sh_opps.size();
						// [game access]
					}

					auto sh_compare = [&](const std::vector<decap_shadow_job> &jobs, char *ma, const std::vector<decap_outcome> &oa, char *mb,
					                      const std::vector<decap_outcome> &ob, size_t slot) {
						for (size_t k = 0; k < jobs.size(); k++) {
							if (!sh_answered(oa, ma, slot, k) || !sh_answered(ob, mb, slot, k)) continue;
							const decap_shadow_meta *a = sh_meta(ma, slot, k), *b = sh_meta(mb, slot, k);
							const bool bot = jobs[k].subj == 0;
							const double *ca = (const double *)(ma + k * slot + sh_slot_meta), *cb = (const double *)(mb + k * slot + sh_slot_meta);
							bool mis = false, trunc = false;
							for (int t = 0; t < 3 && !mis && !trunc; t++) {
								const int pl_b = bot ? b->lop[t] : b->lme[t];
								if (a->ran[t] != b->ran[t] && !(t > 0 && (bot ? b->lop[t - 1] : b->lme[t - 1]))) { mis = true; break; }
								if (!a->ran[t] || !b->ran[t]) break;
								const int nf = std::min(a->nf[t], b->nf[t]);
								// [game access]
								if (pl_b) { trunc = true; break; }
								if (a->nf[t] != b->nf[t] || a->tf[t] != b->tf[t] || (bot ? a->lme[t] != b->lme[t] : a->lop[t] != b->lop[t]) || a->over[t] != b->over[t] ||
								    a->unknown != b->unknown) mis = true;
							}
							if (!trunc && (bot ? a->pts[0] != b->pts[0] : a->pts[1] != b->pts[1])) mis = true;
							sh_vcases++;
							sh_vmis += mis;
							sh_vtrunc += trunc && !mis;
						}
					};
					if (!sh_abort && sh_verify && sh_freeze) {
						std::vector<decap_shadow_job> oj2 = sh_ojobs;
						std::vector<decap_outcome> oo2;
						const long tf = decap_ms_since(t0);
						char *m2 = sh_run(oj2, sh_slot_cap, 0, 1000000000L, oo2);
						sh_ms_opp_free = decap_ms_since(t0) - tf;
						if (m2) sh_compare(oj2, sh_omap, sh_oouts, m2, oo2, sh_slot_cap);
					}

					long sh_gen_ms = 0;
					double sh_best_J = -1e18;
					sh_seq sh_best = {};
					bool sh_have = false;
					if (!sh_abort) {
						decap_shadow_opps = &sh_opps;
						decap_shadow_par = &sh_par;
						float p[3][N_JOINTS][4];
						for (int t = 0; t < 3; t++) for (int j = 0; j < N_JOINTS; j++) for (int s2 = 0; s2 < 4; s2++) p[t][j][s2] = sh_seed.states[j] == s2 + 1 ? 0.4f : 0.2f;
						auto urand = [&]() { return (float)sh_rand() / 16777216.0f; };

						std::vector<std::array<Move, 3>> cl_sseq;
						if (cl.ran && decap_env_int("DECAP_CL_S", 0) == 1 && cl.res.scope[0] == 0) {
							const int k = std::min(sh_pop - 1, std::min(cl.res.ntop, std::max(1, std::min((int)DECAP_CL_TOPK, decap_env_int("DECAP_CL_K", 8)))));
							for (int i = 0; i < k; i++) {
								const decap_cl_seq &q = cl.res.top[i];
								if (!decap_cl_digits_ok(q.m1)) continue;
								std::array<Move, 3> a;
								decap_cl_move(sh_seed, q.m1, a[0]);
								decap_cl_move(sh_seed, decap_cl_digits_ok(q.m2) ? q.m2 : q.m1, a[1]);
								a[2] = a[1];
								cl_sseq.push_back(a);
							}
							cl.s_offered = (int)cl_sseq.size();
						}
						for (int g = 0; g < sh_gens; g++) {
							const long at = decap_ms_since(t0) - sh_start;
							const long hold = std::max(lp_cap * 15 / 100, 2 * sh_gen_ms * sh_keep * (long)theirs.size() / sh_pop);
							const long left = lp_cap - hold - at;
							if (left <= 0) break;
							std::vector<sh_seq> seqs(sh_pop);
							std::vector<decap_shadow_job> jobs(sh_pop);
							const bool g1keep = g == 0 && (sh_dumping || sh_verify);
							for (int i = 0; i < sh_pop; i++) {
								sh_seq &q = seqs[i];
								q.J = -1000.0;
								q.gen = g;
								for (int t = 0; t < 3; t++) {
									q.m[t] = sh_seed;
									if (g == 0 && i == 0) continue;
									for (int j = 0; j < N_JOINTS; j++) {
										// [game access]
										float r = urand() * (p[t][j][0] + p[t][j][1] + p[t][j][2] + p[t][j][3]);
										int s2 = 0;
										while (s2 < 3 && (r -= p[t][j][s2]) >= 0.0f) s2++;
										q.m[t].states[j] = (uint8_t)(s2 + 1);
									}
								}
								q.clx = -1;
								if (g == 0 && i > 0 && i >= sh_pop - (int)cl_sseq.size()) {
									q.clx = i - (sh_pop - (int)cl_sseq.size());
									for (int t = 0; t < 3; t++) q.m[t] = cl_sseq[q.clx][t];
								}
								for (int t = 0; t < 3; t++) jobs[i].bot[t] = q.m[t], jobs[i].opp[t] = sh_op.opp[t];
								jobs[i].subj = 0;
								jobs[i].score = 1;
								jobs[i].keep_cap = g1keep;
								jobs[i].keep_pairs = g == 0 && sh_dumping;
							}
							// [game access]
							std::vector<decap_outcome> outs;
							char *m = sh_run(jobs, slot, sh_freeze, left, outs);
							sh_seqs += sh_pop;
							sh_done++;
							sh_gen_ms = decap_ms_since(t0) - sh_start - at;
							if (g == 0) {
								sh_ms_g1 = sh_gen_ms;
								sh_g1jobs = jobs;
								sh_g1outs = outs;
								sh_g1map = m;
								sh_g1slot = slot;
								sh_g1pu = decap_full.pu;
							}
							std::vector<std::pair<double, int>> rank;
							for (int i = 0; i < sh_pop && m; i++) {
								if (!sh_answered(outs, m, slot, i) || i >= (int)decap_full.pu.size()) continue;
								const uint8_t pu = decap_full.pu[i];
								if (!(pu & 4) || (pu & 1)) continue;
								const decap_shadow_meta *mt = sh_meta(m, slot, i);
								if (mt->fl == 1 || !std::isfinite(mt->J) || !std::isfinite(outs[i].d1)) continue;
								sh_seq &q = seqs[i];
								q.J = mt->J, q.V = mt->V, q.reach = mt->reach, q.close = mt->close, q.neg = mt->neg, q.fl = mt->fl;
								for (int t = 0; t < 3; t++) q.touch[t] = mt->touch[t], q.predD[t] = mt->predD[t];
								sh_all.push_back(q);
								rank.push_back(std::make_pair(q.J, i));
								sh_scored++;
								cl.s_scored += q.clx >= 0;
							}
							if (sh_have) rank.push_back(std::make_pair(sh_best_J, -1));
							std::stable_sort(rank.begin(), rank.end(), [](const std::pair<double, int> &a, const std::pair<double, int> &b) { return a.first > b.first; });
							const int ne = std::min((int)rank.size(), sh_elites);
							if (ne > 0) {
								float freq[3][N_JOINTS][4];
								memset(freq, 0, sizeof(freq));
								for (int e = 0; e < ne; e++) {
									const sh_seq &q = rank[e].second < 0 ? sh_best : seqs[rank[e].second];
									for (int t = 0; t < 3; t++) {
										for (int j = 0; j < N_JOINTS; j++) {
											int st = q.m[t].states[j];
											if (st >= 1 && st <= 4) freq[t][j][st - 1] += 1.0f / ne;
										}
									}
								}
								for (int t = 0; t < 3; t++) {
									for (int j = 0; j < N_JOINTS; j++) {
										float tot = 0.0f;
										for (int s2 = 0; s2 < 4; s2++) {
											p[t][j][s2] = std::max(0.03f, 0.3f * p[t][j][s2] + 0.7f * freq[t][j][s2]);
											tot += p[t][j][s2];
										}
										for (int s2 = 0; s2 < 4; s2++) p[t][j][s2] /= tot;
									}
								}
								if (rank[0].second >= 0) {
									sh_best = seqs[rank[0].second];
									sh_best_J = sh_best.J;
									sh_have = true;
								}
							}
							if (g == 0 && sh_verify && sh_freeze && m) {
								std::vector<decap_shadow_job> j2 = jobs;
								for (decap_shadow_job &x : j2) x.score = 0, x.keep_pairs = 0;
								std::vector<decap_outcome> o2;
								const size_t slot2 = sh_slot_meta + sh_capn * sizeof(double);
								std::vector<decap_shadow_job> j1 = j2;
								std::vector<decap_outcome> o1;
								const long tf = decap_ms_since(t0);
								char *m1 = sh_run(j1, slot2, 1, 1000000000L, o1);
								const long t1 = decap_ms_since(t0);
								char *m2 = sh_run(j2, slot2, 0, 1000000000L, o2);
								sh_ms_g1_free = decap_ms_since(t0) - t1;
								sh_ms_g1 = t1 - tf;
								if (m1 && m2) sh_compare(j2, m1, o1, m2, o2, slot2);
							}
						}
						decap_shadow_opps = nullptr;
						decap_shadow_par = nullptr;
					}
					decap_xp.on = sh_xon;

					std::stable_sort(sh_all.begin(), sh_all.end(), [](const sh_seq &a, const sh_seq &b) { return a.J > b.J; });
					double sh_share1 = NAN, sh_pred1 = NAN;
					if (!sh_all.empty()) {
						double s = 0;
						int n = 0;
						for (size_t k = 0; k < sh_all.size() && n < sh_top; k++, n++) s += sh_all[k].touch[0];
						sh_share1 = s / n;
						sh_pred1 = sh_all[0].predD[0];
					}
					std::vector<Move> lp_first;
					for (const sh_seq &q : sh_all) {
						if ((int)lp_first.size() >= sh_keep) break;
						if (seen(q.m[0]) || decap_script_for(q.m[0]) != nullptr) continue;
						decap_script s;
						s.first = q.m[0];
						s.rest.assign(q.m + 1, q.m + 3);
						decap_decision_scripts.push_back(s);
						lp_first.push_back(q.m[0]);
						lp_kj.push_back((float)q.J);
						if (q.clx >= 0) cl.s_kept_first.push_back(q.m[0]), cl.s_kept++;
					}
					if (cl.s_offered > 0) {
						for (size_t k = 0; k < sh_all.size(); k++) if (sh_all[k].clx >= 0) { cl.s_best_rank = (int)k; break; }
						cl.s_nall = (int)sh_all.size();
					}
					int sh_kept_safe = 0, sh_hand = -1;
					if (!lp_first.empty()) {
						std::vector<decap_outcome> outs;
						decap_plan_outcomes = &outs;
						decap_plan_budget_ms = std::max(1L, lp_cap - (decap_ms_since(t0) - sh_start));
						const decap_pick lp_kp = decap_lookahead(seat, lp_first, theirs, n_frames, skill_level, plan_seed);
						decap_plan_budget_ms = -1;
						decap_plan_outcomes = nullptr;
						sh_hand = lp_kp.results;
						lp_c0 = cand.size();
						lp_cn = lp_first.size();
						for (size_t c = 0; c < lp_first.size(); c++) c_pv.push_back(pv_row(decap_full, c));
						for (size_t c = 0; c < lp_first.size(); c++) {
							bool ok = lp_kp.results > 0 &&
							          (full_res ? decap_full.ok(c) : (c < DECAP_DIAG_MAX && decap_diag.why[c] == 0 && std::isfinite(decap_diag.mean_v[c])));
							cand.push_back(lp_first[c]);
							c_safe.push_back(ok);
							c_mean.push_back(ok ? (full_res ? decap_full.mean_v[c] : decap_diag.mean_v[c]) : -9.0f);
							c_min.push_back(ok ? (full_res ? decap_full.min_v[c] : decap_diag.min_v[c]) : -9.0f);
							sh_kept_safe += ok;
						}
						cem_note |= 64 | 256;
					}

					{
						const bool near_ = first.d0 < decap_env_int("DECAP_SHADOW_NEAR_MILLI", 2500) / 1000.0f;
						const bool touch_ = !(sh_share1 < decap_env_int("DECAP_SHADOW_TOUCH_MILLI", 30) / 1000.0);
						const bool arrive_ = !(sh_pred1 >= decap_env_int("DECAP_SHADOW_ARRIVE1_MILLI", 2200) / 1000.0);
						static char gate[64];
						snprintf(gate, sizeof(gate), "%s%s%s", near_ ? " near" : "", touch_ ? " touch" : "", arrive_ ? " arrive1" : "");
						sh_gate = gate;
						lp_bonus_ok = !(near_ || touch_ || arrive_);
						if (!lp_bonus_ok && lp_cn > 0) cem_note |= 512;
					}
					lp_ms = std::max(1L, decap_ms_since(t0) - sh_start);
					snprintf(line, sizeof(line),
					         // [game access: 10 lines]
					std::string sl(line);
					if (sh_verify) {
						char v[300];
						// [game access: 2 lines]
						sl += v;
					}
					sl += "\n";
					decap_sh_log(sl.c_str());

					if (sh_dumping && sh_omap) {
						char path[700];
						// [game access]
						FILE *f = fopen(path, "wb");
						if (f) {
							auto wi = [&](int32_t x) { fwrite(&x, 4, 1, f); };
							auto wd = [&](double x) { fwrite(&x, 8, 1, f); };
							auto wm = [&](const Move &r) { fwrite(r.states, 1, N_JOINTS, f); };
							// [game access]
							wi(sh_g1map ? (int32_t)sh_g1jobs.size() : 0); wi(sh_freeze); wi(decap_env_int("DECAP_SHADOW_HC", 0));
							// [game access]
							wd(sh_par.d0); wd(sh_par.rw); wd(sh_par.gamma); wd(sh_par.wc2); wd(sh_par.wc3); wd(sh_par.margin); wd(sh_par.pen2); wd(sh_par.pen3);
							for (int t = 0; t < 3; t++) wm(sh_bp.bot[t]);
							for (int t = 0; t < 3; t++) wm(sh_op.opp[t]);
							std::vector<int> kidx(sh_ojobs.size(), -1);
							for (size_t k = 0; k < sh_okeep.size(); k++) kidx[sh_okeep[k]] = (int)k;
							for (size_t k = 0; k < sh_ojobs.size(); k++) {
								for (int t = 0; t < 3; t++) wm(sh_ojobs[k].opp[t]);
								wi(sh_answered(sh_oouts, sh_omap, sh_slot_cap, k));
								wi(kidx[k]);
								for (int t = 0; t < 3; t++) wi(kidx[k] >= 0 ? sh_opps[kidx[k]].valid[t] : 0);
								fwrite(sh_omap + k * sh_slot_cap, 1, sh_slot_cap, f);
							}
							for (size_t i = 0; sh_g1map && i < sh_g1jobs.size(); i++) {
								for (int t = 0; t < 3; t++) wm(sh_g1jobs[i].bot[t]);
								wi(sh_answered(sh_g1outs, sh_g1map, sh_g1slot, i));
								wi(i < sh_g1pu.size() ? sh_g1pu[i] : 0);
								fwrite(sh_g1map + i * sh_g1slot, 1, sh_g1slot, f);
							}
							fclose(f);
						}
					}
					// [game access]
				}
			}

			auto lp_cut = [&](long num, long den) { return lp_ms + (budget_ms - lp_ms) * num / den; };
			auto lp_is = [&](size_t c) { return c >= lp_c0 && c < lp_c0 + lp_cn; };
			if (cem > 0 && !race_dec && decap_ms_since(t0) < lp_cut(4, 10)) {
				static const int units[][3] = {
					{0, -1, -1}, {1, -1, -1}, {2, -1, -1}, {3, -1, -1}, {4, -1, -1}, {5, -1, -1}, {6, -1, -1}, {7, -1, -1},
					{8, -1, -1}, {9, -1, -1}, {10, -1, -1}, {11, -1, -1}, {12, -1, -1}, {13, -1, -1}, {14, -1, -1},
					{15, -1, -1}, {16, -1, -1}, {17, -1, -1}, {18, -1, -1}, {19, -1, -1},
					{4, 7, -2}, {5, 8, -2}, {6, 9, -2}, {10, 11, -2}, {12, 13, -2}, {14, 15, -2}, {16, 17, -2}, {18, 19, -2},
					{14, 16, 18}, {15, 17, 19}, {4, 5, 6}, {7, 8, 9} };
				const int n_units = (int)(sizeof(units) / sizeof(units[0]));
				// [game access]
				int gens = decap_env_int("DECAP_CEM_GENS", 1);
				auto draw_state = [&](int j, int cur) {
					float w[4], t = 0.0f;
					for (int s2 = 0; s2 < 4; s2++) {
						w[s2] = (s2 + 1 == cur) ? 0.0f : std::max(0.02f, have_probs ? probs[j][s2] : 0.25f);
						t += w[s2];
					}
					float r = (float(rand()) / (float(RAND_MAX) + 1.0f)) * t;
					for (int s2 = 0; s2 < 4; s2++) {
						if ((r -= w[s2]) < 0.0f) return s2 + 1;
					}
					return cur == 4 ? 3 : 4;
				};
				for (int gen = 0; gen < gens; gen++) {
					std::vector<std::pair<float, size_t>> ranked;
					for (size_t c = 0; c < cand.size(); c++) {
						if (c_safe[c]) ranked.push_back(std::make_pair(stage_score(c), c));
					}
					std::sort(ranked.begin(), ranked.end(), std::greater<std::pair<float, size_t>>());
					int ne = std::min((int)ranked.size(), gen == 0 ? elites : 3);
					std::vector<Move> muts;
					for (int e = 0; e < ne; e++) {
						const Move par = cand[ranked[e].second];
						for (int m = 0; m < samples; m++) {
							Move r = par;
							if (m == samples - 1 && !similar.empty()) {
								int best = -1, bd = 99;
								for (size_t q = 0; q < similar.size(); q++) {
									int d = 0;
									for (int j = 0; j < N_JOINTS; j++) d += similar[q].states[j] != par.states[j];
									if (d > 0 && d < bd) bd = d, best = (int)q;
								}
								if (best >= 0) {
									int copies = 1 + rand() % 3;
									for (int x = 0; x < copies; x++) {
										int j = rand() % N_JOINTS;
										// [game access]
									}
								}
							}
							else {
								float uw[64], ut = 0.0f;
								for (int u = 0; u < n_units; u++) {
									float w = 0.0f;
									int k = 0;
									for (int x = 0; x < 3; x++) {
										int j = units[u][x];
										// [game access]
										int st = r.states[j];
										w += 1.0f - (have_probs && st >= 1 && st <= 4 ? probs[j][st - 1] : 0.25f);
										k++;
									}
									uw[u] = k ? std::max(0.05f, w / k) : 0.0f;
									ut += uw[u];
								}
								int changes = 1 + rand() % 3;
								for (int x = 0; x < changes && ut > 0.0f; x++) {
									float pr = (float(rand()) / (float(RAND_MAX) + 1.0f)) * ut;
									int u = 0;
									while (u < n_units - 1 && (pr -= uw[u]) >= 0.0f) u++;
									ut -= uw[u];
									uw[u] = 0.0f;
									if (units[u][2] == -2) {
										int a = units[u][0], b = units[u][1];
										// [game access]
										int st = draw_state(a, r.states[a]);
										r.states[a] = r.states[b] = (uint8_t)st;
									}
									else {
										for (int y = 0; y < 3; y++) {
											int j = units[u][y];
											// [game access]
											r.states[j] = (uint8_t)draw_state(j, r.states[j]);
										}
									}
								}
							}
							bool dup = seen(r);
							for (const Move &q : muts) dup |= memcmp(q.states, r.states, sizeof(r.states)) == 0;
							if (!dup) muts.push_back(r);
						}
					}
					if (muts.empty() || plans + (int)(muts.size() * theirs.size()) > plan_budget) break;
					plans += (int)(muts.size() * theirs.size());
					decap_lookahead(seat, muts, theirs, n_frames, skill_level, plan_seed);
					if (decap_env_int("DECAP_CEM_LOG", 0)) {
						int n_ok = 0, n_ok_cap = 0;
						for (size_t c = 0; c < muts.size(); c++) {
							n_ok += decap_full.ok(c);
							n_ok_cap += c < DECAP_DIAG_MAX && decap_full.ok(c);
						}
						const char *dir = getenv("DECAP_DECISION_LOG");
						if (dir && *dir) {
							char path[600], line[200];
							// [game access]
							int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
							if (fd >= 0) {
								// [game access]
								ssize_t w = write(fd, line, len);
								(void)w;
								close(fd);
							}
						}
					}
					for (size_t c = 0; c < muts.size(); c++) c_pv.push_back(pv_row(decap_full, c));
					for (size_t c = 0; c < muts.size(); c++) {
						if (rescue_cem) {
							// [game access]
							cand.push_back(muts[c]);
							c_safe.push_back(r > -9.0f);
							c_mean.push_back(r);
							c_min.push_back(r);
							continue;
						}
						bool ok = full_res ? decap_full.ok(c) : (c < DECAP_DIAG_MAX && decap_diag.why[c] == 0 && std::isfinite(decap_diag.mean_v[c]));
						cand.push_back(muts[c]);
						c_safe.push_back(ok);
						c_mean.push_back(ok ? (full_res ? decap_full.mean_v[c] : decap_diag.mean_v[c]) : -9.0f);
						c_min.push_back(ok ? (full_res ? decap_full.min_v[c] : decap_diag.min_v[c]) : -9.0f);
					}
					if (decap_publish_to) {
						size_t bi = cand.size();
						float bs = -9.0f;
						for (size_t c = 0; c < cand.size(); c++) {
							if (c_safe[c] && stage_score(c) > bs) bs = stage_score(c), bi = c;
						}
						if (bi < cand.size()) decap_publish(cand[bi], bs, 2);
					}
					if (decap_ms_since(t0) >= lp_cut(4, 10)) break;
				}
			}

			std::vector<size_t> order;
			for (size_t c = 0; c < cand.size(); c++) {
				if (c_safe[c]) order.push_back(c);
			}
			std::sort(order.begin(), order.end(), [&](size_t a, size_t b) { return stage_score(a) > stage_score(b); });
			std::vector<size_t> fin(order.begin(), order.begin() + std::min(order.size(), (size_t)std::max(1, n_final)));

			if (lp_cn > 0 && !rescue_cem && decap_env_int("DECAP_LP_FINAL", 0)) {
				for (size_t c = lp_c0; c < lp_c0 + lp_cn && c < cand.size(); c++) {
					if (c_safe[c] && std::find(fin.begin(), fin.end(), c) == fin.end()) fin.push_back(c);
				}
			}
			std::vector<float> fscore(fin.size()), fmin(fin.size());
			std::vector<char> fsafe(fin.size(), 1);

			std::vector<float> fmean(fin.size());
			std::vector<int> fcnt(fin.size(), (int)theirs.size());
			std::vector<std::vector<float>> held_pv(fin.size()), adv_pv(fin.size());
			std::vector<Move> adv_seen;
			for (size_t k = 0; k < fin.size(); k++) {
				fscore[k] = stage_score(fin[k]) + anchor * logprob(cand[fin[k]]);
				fmin[k] = c_min[fin[k]];
				fmean[k] = c_mean[fin[k]];
			}
			decap_lf_snap(0, fscore, fsafe, fmin);
			const bool adv_worst = decap_env_int("DECAP_ADV_WORST", 0) != 0;
			const float noans_pen = decap_env_int("DECAP_ADV_NOANS_MILLI", 50) / 1000.0f;
			std::vector<Move> held;
			if (n_final > 0 && !fin.empty() && !rescue_cem && decap_ms_since(t0) < lp_cut(7, 10)) {
				auto add_held = [&](const Move &r) {
					for (const Move &q : theirs) {
						if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return;
					}
					for (const Move &q : held) {
						if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return;
					}
					held.push_back(r);
				};
				// [game access]
				decap_xp_label(held, 0, 4);
				size_t h1 = held.size();
				// [game access]
				decap_xp_label(held, h1, 3);
				size_t h2 = held.size();
				// [game access]
				decap_xp_label(held, h2, 2);
				if (!held.empty()) {
					std::vector<Move> fm;
					for (size_t k : fin) fm.push_back(cand[k]);
					decap_lookahead(seat, fm, held, n_frames, skill_level, plan_seed ^ 0x9e3779b9u);
					size_t nt = theirs.size(), nh = held.size();
					int any = 0;
					for (size_t k = 0; k < fin.size(); k++) held_pv[k] = pv_row(decap_full, k);
					for (size_t k = 0; k < fin.size(); k++) {
						bool ok = full_res ? decap_full.ok(k) : (k < DECAP_DIAG_MAX && decap_diag.why[k] == 0 && std::isfinite(decap_diag.mean_v[k]));
						fsafe[k] = ok;
						any += ok;
						if (ok) {
							float h_mean = full_res ? decap_full.mean_v[k] : decap_diag.mean_v[k];
							float h_min = full_res ? decap_full.min_v[k] : decap_diag.min_v[k];
							float mean_all = (nt * c_mean[fin[k]] + nh * h_mean) / (float)(nt + nh);
							float min_all = std::min(c_min[fin[k]], h_min);
							fscore[k] = (1.0f - xw_worst) * mean_all + xw_worst * min_all + anchor * logprob(cand[fin[k]]);
							fmin[k] = min_all;
							fmean[k] = mean_all;
							fcnt[k] = (int)(nt + nh);
						}
					}
					if (!any) std::fill(fsafe.begin(), fsafe.end(), 1);
					if (!any && adv_worst) {
						for (size_t k = 0; k < fin.size() && k < decap_full.why.size(); k++) {
							int hg = decap_full.n_ans[k];
							if (hg > 0) {
								float mean_all = (nt * c_mean[fin[k]] + hg * decap_full.mean_all[k]) / (float)(nt + hg);
								float min_all = std::min(c_min[fin[k]], decap_full.min_all[k]);
								fscore[k] = (1.0f - xw_worst) * mean_all + xw_worst * min_all + anchor * logprob(cand[fin[k]]);
								fmin[k] = min_all;
								fmean[k] = mean_all;
								fcnt[k] = (int)(nt + hg);
							}
							else {
								fscore[k] -= noans_pen;
							}
						}
					}
					cem_note |= 2;
				}
			}

			decap_lf_snap(1, fscore, fsafe, fmin);
			int n_adv = decap_env_int("DECAP_ADV_REPLIES", 0);

			const bool adv_dedupe = decap_env_int("DECAP_ADV_DEDUPE", 0) != 0;
			if (n_adv > 0 && !fin.empty() && !rescue_cem && !race_dec && decap_ms_since(t0) < lp_cut(7, 10)) {
				std::vector<Move> adv;
				auto add_adv = [&](const Move &r) {
					for (const Move &q : theirs) {
						if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return;
					}
					if (adv_dedupe) {
						for (const Move &q : held) {
							if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return;
						}
					}
					for (const Move &q : adv) {
						if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return;
					}
					adv.push_back(r);
				};
				float adv_tau = std::max(1, decap_env_int("DECAP_ADV_TAU_MILLI", 300)) / 1000.0f;
				// [game access]
				decap_xp_label(adv, 0, 5);
				int adv_knn = decap_env_int("DECAP_ADV_KNN", 2);
				if (adv_knn > 0) {
					size_t a1 = adv.size();
					if (adv_dedupe) {
						int added = 0;
						// [game access]
							if (added >= adv_knn) break;
							size_t before = adv.size();
							add_adv(r);
							added += adv.size() > before ? 1 : 0;
						}
					}
					else {
						// [game access]
					}
					decap_xp_label(adv, a1, 3);
				}
				int adv_cb = decap_env_int("DECAP_ADV_COMEBACK", 0);
				if (adv_cb > 0) {
					size_t a2 = adv.size();
					// [game access]
					decap_xp_label(adv, a2, 6);
				}
				if (!adv.empty()) {
					std::vector<Move> fm;
					for (size_t k : fin) fm.push_back(cand[k]);
					decap_lookahead(seat, fm, adv, n_frames, skill_level, plan_seed ^ 0x3c6ef372u);
					adv_seen = adv;
					for (size_t k = 0; k < fin.size(); k++) adv_pv[k] = pv_row(decap_full, k);
					for (size_t k = 0; k < fin.size() && (full_res || k < DECAP_DIAG_MAX); k++) {

						float w = full_res ? (k < decap_full.why.size() ? (decap_full.why[k] == 0 ? decap_full.min_v[k] : decap_full.value[k]) : NAN)
						                   : (decap_diag.why[k] == 0 ? decap_diag.min_v[k] : decap_diag.value[k]);
						if (adv_worst && k < decap_full.why.size()) {

							w = decap_full.why[k] == 0 ? decap_full.min_v[k] : decap_full.min_all[k];
							if (!std::isfinite(w)) fscore[k] -= noans_pen;
						}
						if (std::isfinite(w) && w < fmin[k]) {
							fscore[k] -= xw_worst * (fmin[k] - w);
							fmin[k] = w;
						}
					}
					cem_note |= 4;
				}
			}

			decap_lf_snap(2, fscore, fsafe, fmin);
			// [game access]
				memset(&decap_cs, 0, sizeof(decap_cs));
				decap_cs.magic = 0x31435058u;
				decap_cs.ms_start = (int32_t)decap_ms_since(t0);
				decap_cs.d0 = first.d0;

				long cs_ms = lp_cut(std::max(0, decap_env_int("DECAP_OPP_CEM_STOP_PERMILLE", 800)), 1000);
				long cs_hard = lp_cut(std::max(0, decap_env_int("DECAP_OPP_CEM_HARD_PERMILLE", 920)), 1000);
				// [game access]
				if (decap_env_int("DECAP_OPP_CEM_HARD_MS", 0) > 0) cs_hard = std::min(cs_hard, (long)decap_env_int("DECAP_OPP_CEM_HARD_MS", 0));
				if (decap_env_int("DECAP_OPP_CEM_STAGE_MS", 0) > 0) {
					long margin = std::max(0L, cs_hard - cs_ms);
					cs_ms = decap_cs.ms_start + decap_env_int("DECAP_OPP_CEM_STAGE_MS", 0);
					cs_hard = cs_ms + margin;
				}
				// [game access]
				const int cs_gens = std::max(0, decap_env_int("DECAP_OPP_CEM_GENS", 2)), cs_elites = std::max(1, decap_env_int("DECAP_OPP_CEM_ELITES", 6));
				const int cs_maxchg = std::max(1, decap_env_int("DECAP_OPP_CEM_MAXCHG", 5)), cs_keep = std::max(1, decap_env_int("DECAP_OPP_CEM_KEEP", 2));
				const int cs_mode = decap_env_int("DECAP_OPP_CEM_MODE", 0), cs_rounds = std::max(1, decap_env_int("DECAP_OPP_CEM_ROUNDS", 3));
				const float cs_w = std::max(0, decap_env_int("DECAP_OPP_CEM_W_MILLI", 1000)) / 1000.0f;
				const float cs_q = std::min(1000, std::max(0, decap_env_int("DECAP_OPP_CEM_Q_MILLI", 250))) / 1000.0f;
				const float cs_gain = (float)decap_env_int("DECAP_OPP_CEM_MIN_GAIN", 0);
				const bool cs_veto = decap_env_int("DECAP_OPP_CEM_VETO", 0) != 0, cs_share = decap_env_int("DECAP_OPP_CEM_SHARE", 0) != 0;
				const float cs_elite_mix = std::min(1000, std::max(0, decap_env_int("DECAP_OPP_CEM_ELITE_MIX_MILLI", 500))) / 1000.0f;
				const float cs_win_margin = decap_env_int("DECAP_OPP_CEM_WIN_MARGIN_MILLI", 20) / 1000.0f;
				const int cs_target = decap_env_int("DECAP_OPP_CEM_TARGET", 1);
				const float cs_ghost_w = std::min(1000, std::max(0, decap_env_int("DECAP_OPP_CEM_GHOST_MILLI", 250))) / 1000.0f;
				const float cs_mix_tau = std::max(1, decap_env_int("DECAP_OPP_CEM_MIX_TAU_MILLI", 50)) / 1000.0f;

				const float cs_eps = std::min(1000, std::max(0, decap_env_int("DECAP_OPP_CEM_EPS_MILLI", 0))) / 1000.0f;
				const float cs_cap = std::max(0, decap_env_int("DECAP_OPP_CEM_CAP_MILLI", 0)) / 1000.0f;
				const float cs_lp_min = (getenv("DECAP_OPP_CEM_LOGP_MIN_MILLI") ? decap_env_int("DECAP_OPP_CEM_LOGP_MIN_MILLI", 0)
				                                                       : decap_env_int("DECAP_OPP_CEM_LP_MIN_MILLI", -1800)) / 1000.0f;
				const int cs_keep_mix = std::max(1, decap_env_int("DECAP_OPP_CEM_KEEP_MIX", 4));
				const int op = 1 - seat;
				const size_t nf = fin.size();
				auto left_ms = [&]() { return cs_hard - decap_ms_since(t0); };

				std::vector<Move> seeds;
				auto add_seed = [&](const Move &r) {
					for (const Move &q : seeds) {
						if (memcmp(q.states, r.states, sizeof(r.states)) == 0) return;
					}
					seeds.push_back(r);
				};
				for (const Move &r : theirs) add_seed(r);
				for (const Move &r : held) add_seed(r);
				for (const Move &r : adv_seen) add_seed(r);
				const size_t n_orig = seeds.size();

				const bool cs_go = first.d0 <= decap_env_int("DECAP_OPP_CEM_NEAR_MILLI", 3000) / 1000.0f && decap_ms_since(t0) < cs_ms && left_ms() >= 500;
				if (cs_go && decap_env_int("DECAP_OPP_CEM_SEED_KNN", 6) > 0 && !seeds.empty()) {
					int want = decap_env_int("DECAP_OPP_CEM_SEED_KNN", 6), got = 0;
					int skip = decap_env_int("DECAP_KNN_REPLIES", 0) + 2 + decap_env_int("DECAP_ADV_KNN", 2);
					// [game access]
						if (got >= want) break;
						size_t before = seeds.size();
						add_seed(r);
						got += seeds.size() > before ? 1 : 0;
					}
				}
				if (cs_go && decap_env_int("DECAP_OPP_CEM_SEED_DRAWS", 4) > 0 && !seeds.empty()) {
					// [game access]
				}

				auto softq = [&](std::vector<std::pair<float, float>> vw) {
					std::sort(vw.begin(), vw.end());
					float tot = 0.0f;
					for (const auto &x : vw) tot += x.second;
					if (vw.empty() || tot <= 0.0f) return NAN;
					float target = std::max(cs_q * tot, 1e-6f), got = 0.0f, s = 0.0f;
					for (const auto &x : vw) {
						float take = std::min(x.second, target - got);
						if (take <= 0.0f) break;
						s += take * x.first;
						got += take;
					}
					return got > 0.0f ? s / got : vw[0].first;
				};

				auto reply_vals = [&](size_t k) {
					std::vector<std::pair<float, float>> vw;
					if (fin[k] < c_pv.size()) for (float v : c_pv[fin[k]]) if (std::isfinite(v)) vw.push_back(std::make_pair(v, 1.0f));
					if (fcnt[k] > (int)theirs.size()) for (float v : held_pv[k]) if (std::isfinite(v)) vw.push_back(std::make_pair(v, 1.0f));
					for (float v : adv_pv[k]) if (std::isfinite(v)) vw.push_back(std::make_pair(v, 1.0f));
					return vw;
				};
				std::vector<float> base(fscore), delta(nf, 0.0f), q0(nf, NAN);
				if (cs_mode == 2) {
					for (size_t k = 0; k < nf; k++) {
						q0[k] = softq(reply_vals(k));
						if (std::isfinite(q0[k]) && std::isfinite(fmin[k])) base[k] = fscore[k] + xw_worst * (q0[k] - fmin[k]);
					}
				}
				std::vector<char> attacked(nf, 0), vetoed(nf, 0);
				int kb0 = -1;
				for (size_t k = 0; k < nf; k++) {
					if (fsafe[k] && (kb0 < 0 || fscore[k] > fscore[kb0])) kb0 = (int)k;
				}
				float oprobs[N_JOINTS][4];
				// [game access]
				static const int cs_units[][3] = {
					{0, -1, -1}, {1, -1, -1}, {2, -1, -1}, {3, -1, -1}, {4, -1, -1}, {5, -1, -1}, {6, -1, -1}, {7, -1, -1},
					{8, -1, -1}, {9, -1, -1}, {10, -1, -1}, {11, -1, -1}, {12, -1, -1}, {13, -1, -1}, {14, -1, -1},
					{15, -1, -1}, {16, -1, -1}, {17, -1, -1}, {18, -1, -1}, {19, -1, -1},
					{4, 7, -2}, {5, 8, -2}, {6, 9, -2}, {10, 11, -2}, {12, 13, -2}, {14, 15, -2}, {16, 17, -2}, {18, 19, -2},
					{14, 16, 18}, {15, 17, 19}, {4, 5, 6}, {7, 8, 9} };
				const int cs_nu = (int)(sizeof(cs_units) / sizeof(cs_units[0]));
				auto frand = []() { return float(rand()) / (float(RAND_MAX) + 1.0f); };
				auto hamming = [](const Move &a, const Move &b) {
					int d = 0;
					for (int j = 0; j < N_JOINTS; j++) d += a.states[j] != b.states[j];
					return d;
				};
				int status = 4, rounds = 0, turn_sims = 0, full_plans = 0, answered = 0, kept_n = 0;
				bool cut = false;
				float guess_taken = NAN, counter_taken = NAN, guess_j = NAN, counter_j = NAN;
				if (first.d0 > decap_env_int("DECAP_OPP_CEM_NEAR_MILLI", 3000) / 1000.0f) status = 2;
				else if (decap_ms_since(t0) >= cs_ms || left_ms() < 500) status = 3;
				else if (kb0 >= 0 && !seeds.empty()) {
					status = 1;
					decap_publish(cand[fin[kb0]], stage_score(fin[kb0]), 3);
					const bool xon = decap_xp.on;
					for (int round = 0; round < cs_rounds; round++) {

						std::vector<size_t> tgt;
						{
							float best_att = -1e30f;
							for (size_t k = 0; k < nf; k++) {
								if (attacked[k] && fsafe[k] && !vetoed[k]) best_att = std::max(best_att, base[k] + delta[k]);
							}
							std::vector<std::pair<float, size_t>> r;
							for (size_t k = 0; k < nf; k++) {
								if (fsafe[k] && !attacked[k] && (round == 0 || base[k] >= best_att - cs_win_margin)) r.push_back(std::make_pair(base[k], k));
							}
							std::sort(r.begin(), r.end(), std::greater<std::pair<float, size_t>>());
							for (size_t i = 0; i < r.size() && (int)i < cs_k; i++) tgt.push_back(r[i].second);
						}
						if (tgt.empty()) break;
						if (decap_ms_since(t0) >= cs_ms || left_ms() < 500) {
							cut = true;
							break;
						}
						rounds++;

						std::vector<Move> fm;
						std::vector<std::vector<std::pair<size_t, float>>> grp;
						if (cs_target == 0) {
							for (size_t a = 0; a < tgt.size(); a++) {
								fm.push_back(cand[fin[tgt[a]]]);
								grp.push_back(std::vector<std::pair<size_t, float>>(1, std::make_pair(a, 1.0f)));
							}
						}
						else {
							std::vector<std::pair<size_t, float>> g1;
							float gw = cs_target == 2 ? 1.0f : cs_ghost_w;
							if (cs_target == 1) {
								float mx = -1e30f, tot = 0.0f;
								for (size_t k : tgt) mx = std::max(mx, base[k]);
								std::vector<float> w;
								for (size_t k : tgt) w.push_back(expf((base[k] - mx) / cs_mix_tau)), tot += w.back();
								for (size_t a = 0; a < tgt.size(); a++) {
									fm.push_back(cand[fin[tgt[a]]]);
									g1.push_back(std::make_pair(a, (1.0f - gw) * w[a] / std::max(1e-9f, tot)));
								}
							}
							size_t gi = fm.size();
							for (size_t a = 0; a < fm.size(); a++) {
								if (memcmp(fm[a].states, keep.states, sizeof(keep.states)) == 0) gi = a;
							}
							if (gi == fm.size()) fm.push_back(keep);
							bool merged = false;
							for (auto &x : g1) if (x.first == gi) x.second += gw, merged = true;
							if (!merged) g1.push_back(std::make_pair(gi, gw));
							grp.push_back(g1);
						}
						const size_t nr = fm.size(), na = grp.size(), ns = seeds.size();

						auto gval = [&](size_t g, const std::vector<float> &v, size_t nu, size_t u) {
							float s = 0.0f;
							for (const auto &x : grp[g]) {
								size_t i = x.first * nu + u;
								float y = i < v.size() ? v[i] : NAN;
								if (!std::isfinite(y)) return NAN;
								s += x.second * y;
							}
							return s;
						};
						std::vector<std::vector<Move>> pool(na);
						std::vector<std::vector<float>> pjv(na), pdv(na);
						std::vector<std::vector<char>> isseed(na);

						decap_xp.on = false;
						decap_plan_turn_only = 1;
						decap_plan_budget_ms = std::max(1L, std::min(cs_ms - decap_ms_since(t0), left_ms()));
						decap_lookahead(seat, fm, seeds, n_frames, skill_level, plan_seed ^ 0x51ed270bu);
						turn_sims += (int)(nr * ns);
						for (size_t a = 0; a < na; a++) {
							for (size_t t = 0; t < ns; t++) {
								float jv = gval(a, decap_full.pj, ns, t);
								pool[a].push_back(seeds[t]);
								pjv[a].push_back(std::isfinite(jv) ? jv : -1e30f);
								pdv[a].push_back(gval(a, decap_full.pd, ns, t));
								isseed[a].push_back(t < n_orig ? 1 : 0);
							}
						}
						for (int g = 0; g < cs_gens; g++) {
							if (decap_ms_since(t0) >= cs_ms || left_ms() < 500) {
								cut = true;
								break;
							}
							std::vector<Move> U;
							std::vector<size_t> owner;
							for (size_t a = 0; a < na; a++) {
								std::vector<size_t> el;
								for (size_t i = 0; i < pool[a].size(); i++) if (pjv[a][i] > -1e29f) el.push_back(i);
								std::sort(el.begin(), el.end(), [&](size_t x, size_t y) { return pjv[a][x] > pjv[a][y]; });
								if (el.size() > (size_t)cs_elites) el.resize(cs_elites);
								if (el.empty()) continue;
								const int ne = (int)el.size();
								float mix[N_JOINTS][4];
								for (int j = 0; j < N_JOINTS; j++) {
									float fq[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
									for (size_t e : el) {
										int st = pool[a][e].states[j];
										if (st >= 1 && st <= 4) fq[st - 1] += 1.0f / ne;
									}
									for (int s2 = 0; s2 < 4; s2++) mix[j][s2] = cs_elite_mix * fq[s2] + (1.0f - cs_elite_mix) * (have_op ? oprobs[j][s2] : 0.25f);
								}
								auto draw_st = [&](int j, int cur) {
									float w[4], t = 0.0f;
									for (int s2 = 0; s2 < 4; s2++) {
										w[s2] = (s2 + 1 == cur) ? 0.0f : std::max(0.02f, mix[j][s2]);
										t += w[s2];
									}
									float r = frand() * t;
									for (int s2 = 0; s2 < 4; s2++) {
										if ((r -= w[s2]) < 0.0f) return s2 + 1;
									}
									return cur == 4 ? 3 : 4;
								};
								int made = 0;
								size_t u0 = U.size();
								for (int attempt = 0; attempt < 4 * cs_samples && made < cs_samples; attempt++) {
									int tw = ne * (ne + 1) / 2, pr = 0;
									int x = (int)(frand() * tw);
									while (pr < ne - 1 && (x -= (ne - pr)) >= 0) pr++;
									Move r = pool[a][el[pr]];
									bool chg[N_JOINTS] = {};
									if (ne > 1 && rand() % 4 == 0) {
										int o2 = rand() % (ne - 1);
										if (o2 >= pr) o2++;
										const Move &other = pool[a][el[o2]];
										int copies = 1 + rand() % 3;
										for (int c2 = 0; c2 < copies; c2++) {
											int u = rand() % cs_nu;
											for (int y = 0; y < 3; y++) {
												int j = cs_units[u][y];
												// [game access]
												if (r.states[j] != other.states[j]) chg[j] = true;
												r.states[j] = other.states[j];
											}
										}
									}
									else {
										float uw[64], ut = 0.0f;
										for (int u = 0; u < cs_nu; u++) {
											float w = 0.0f;
											int k2 = 0;
											for (int y = 0; y < 3; y++) {
												int j = cs_units[u][y];
												// [game access]
												int st = r.states[j];
												w += 1.0f - (st >= 1 && st <= 4 ? mix[j][st - 1] : 0.25f);
												k2++;
											}
											uw[u] = k2 ? std::max(0.05f, w / k2) : 0.0f;
											ut += uw[u];
										}
										int changes = 1 + rand() % 3;
										for (int c2 = 0; c2 < changes && ut > 0.0f; c2++) {
											float pu = frand() * ut;
											int u = 0;
											while (u < cs_nu - 1 && (pu -= uw[u]) >= 0.0f) u++;
											ut -= uw[u];
											uw[u] = 0.0f;
											if (cs_units[u][2] == -2) {
												int ja = cs_units[u][0], jb = cs_units[u][1];
												// [game access]
												int st = draw_st(ja, r.states[ja]);
												r.states[ja] = r.states[jb] = (uint8_t)st;
												chg[ja] = chg[jb] = true;
											}
											else {
												for (int y = 0; y < 3; y++) {
													int j = cs_units[u][y];
													// [game access]
													r.states[j] = (uint8_t)draw_st(j, r.states[j]);
													chg[j] = true;
												}
											}
										}
									}

									size_t ns_i = 0;
									int dn = 99;
									for (size_t s = 0; s < ns; s++) {
										int d = hamming(r, seeds[s]);
										if (d < dn) dn = d, ns_i = s;
									}
									for (int pass = 0; pass < 2 && dn > cs_maxchg; pass++) {
										int diff[N_JOINTS], nd = 0;
										for (int j = 0; j < N_JOINTS; j++) {
											if (r.states[j] != seeds[ns_i].states[j] && (pass == 1 || !chg[j])) diff[nd++] = j;
										}
										while (nd > 0 && dn > cs_maxchg) {
											int i = rand() % nd;
											r.states[diff[i]] = seeds[ns_i].states[diff[i]];
											diff[i] = diff[--nd];
											dn--;
										}
									}
									if (cs_lp_min < 0.0f && have_op) {
										float lp = 0.0f;
										for (int j = 0; j < N_JOINTS; j++) {
											int st = r.states[j];
											lp += logf(std::max(1e-6f, st >= 1 && st <= 4 ? oprobs[j][st - 1] : 1e-6f));
										}
										if (lp / N_JOINTS < cs_lp_min) continue;
									}
									bool dup = false;
									for (const Move &q : pool[a]) dup = dup || memcmp(q.states, r.states, sizeof(r.states)) == 0;
									for (size_t i = u0; i < U.size(); i++) dup = dup || memcmp(U[i].states, r.states, sizeof(r.states)) == 0;
									if (dup) continue;
									U.push_back(r);
									owner.push_back(a);
									made++;
								}
							}
							if (U.empty()) break;
							const size_t nu = U.size();
							std::vector<char> m(nr * nu, 0);
							for (size_t u = 0; u < nu; u++) {
								for (const auto &x : grp[owner[u]]) m[x.first * nu + u] = 1;
							}
							decap_plan_mask = &m;
							decap_plan_budget_ms = std::max(1L, std::min(cs_ms - decap_ms_since(t0), left_ms()));
							decap_lookahead(seat, fm, U, n_frames, skill_level, plan_seed ^ 0x51ed270bu);
							decap_plan_mask = nullptr;
							for (char x : m) turn_sims += x != 0;
							for (size_t u = 0; u < nu; u++) {
								size_t a = owner[u];
								float jv = gval(a, decap_full.pj, nu, u);
								pool[a].push_back(U[u]);
								pjv[a].push_back(std::isfinite(jv) ? jv : -1e30f);
								pdv[a].push_back(gval(a, decap_full.pd, nu, u));
								isseed[a].push_back(0);
							}
							{
								char line[512];
								// [game access]
								for (size_t a = 0; a < na && len < 440; a++) {
									float b = -1e30f;
									for (float v : pjv[a]) b = std::max(b, v);
									len += snprintf(line + len, sizeof(line) - len, " %.0f", b);
								}
								snprintf(line + len, sizeof(line) - len, " (%ld ms)\n", decap_ms_since(t0));
								decap_cs_log(line);
							}
						}
						decap_plan_turn_only = 0;
						decap_plan_budget_ms = -1;
						decap_xp.on = xon;

						std::vector<Move> U2;
						std::vector<size_t> own2;
						for (size_t a = 0; a < na; a++) {
							float sj = -1e30f;
							for (size_t i = 0; i < pool[a].size(); i++) {
								if (isseed[a][i] && pjv[a][i] > sj) sj = pjv[a][i];
							}
							std::vector<size_t> c2;
							for (size_t i = 0; i < pool[a].size(); i++) {
								if (!isseed[a][i] && pjv[a][i] > -1e29f && pjv[a][i] > sj + cs_gain) c2.push_back(i);
							}
							std::sort(c2.begin(), c2.end(), [&](size_t x, size_t y) { return pjv[a][x] > pjv[a][y]; });
							if (round == 0 && a == 0) {
								float gd = -1e30f, cd = -1e30f, bj = -1e30f;
								for (size_t i = 0; i < pool[a].size(); i++) {
									float dv = std::isfinite(pdv[a][i]) ? pdv[a][i] : -1e30f;
									if (isseed[a][i]) gd = std::max(gd, dv);
									else cd = std::max(cd, dv), bj = std::max(bj, pjv[a][i]);
								}
								guess_j = sj > -1e29f ? sj : NAN;
								counter_j = bj > -1e29f ? bj : NAN;
								guess_taken = gd > -1e29f ? gd : NAN;
								counter_taken = cd > -1e29f ? cd : NAN;
							}
							for (size_t i = 0; i < c2.size() && (int)i < (cs_target == 0 ? cs_keep : cs_keep_mix); i++) {
								U2.push_back(pool[a][c2[i]]);
								own2.push_back(a);
							}
						}
						kept_n += (int)U2.size();
						for (size_t a = 0; a < na; a++) {
							float sj = -1e30f, cj = -1e30f, sd = -1e30f, cd = -1e30f;
							int nsmp = 0;
							for (size_t i = 0; i < pool[a].size(); i++) {
								float dv = std::isfinite(pdv[a][i]) ? pdv[a][i] : -1e30f;
								if (isseed[a][i]) sj = std::max(sj, pjv[a][i]), sd = std::max(sd, dv);
								else cj = std::max(cj, pjv[a][i]), cd = std::max(cd, dv), nsmp++;
							}
							int nk = 0;
							for (size_t u = 0; u < own2.size(); u++) nk += own2[u] == a;
							char line[300];
							// [game access: 2 lines]
							decap_cs_log(line);
						}

						std::vector<Move> fm2;
						std::vector<size_t> rowfin;
						if (cs_target == 0) {
							fm2 = fm;
							rowfin = tgt;
						}
						else {
							for (size_t k = 0; k < nf; k++) fm2.push_back(cand[fin[k]]), rowfin.push_back(k);
						}
						const size_t nr2 = fm2.size();
						std::vector<std::vector<std::pair<float, float>>> cv(nr2);
						std::vector<char> cu(nr2, 0);
						if (!U2.empty() && left_ms() < 300) cut = true;
						if (!U2.empty() && left_ms() >= 300) {
							const size_t nu = U2.size();
							std::vector<char> m(nr2 * nu, 1);
							if (cs_target == 0) {
								for (size_t a = 0; a < nr2; a++) {
									for (size_t u = 0; u < nu; u++) m[a * nu + u] = cs_share || own2[u] == a;
								}
							}
							for (char x : m) full_plans += x != 0;
							decap_xp_label(U2, 0, 7);
							decap_plan_mask = &m;
							decap_plan_budget_ms = std::max(1L, left_ms());
							decap_lookahead(seat, fm2, U2, n_frames, skill_level, plan_seed ^ 0x2545f491u);
							decap_plan_mask = nullptr;
							decap_plan_budget_ms = -1;
							for (size_t a = 0; a < nr2; a++) {
								for (size_t u = 0; u < nu; u++) {
									size_t i = a * nu + u;
									if (!m[i] || i >= decap_full.pu.size() || !(decap_full.pu[i] & 4)) continue;
									answered++;
									cv[a].push_back(std::make_pair(decap_full.pv[i], cs_w));
									if (decap_full.pu[i] & 1) cu[a] = 1;
								}
							}
						}

						std::vector<char> rowok(nr2, 1);
						if (cs_target != 0) {
							for (size_t a = 0; a < nr2; a++) {
								if (cv[a].size() < U2.size()) rowok[a] = 0, cut = true;
							}
						}

						for (size_t a = 0; a < nr2; a++) {
							if (!rowok[a]) continue;
							size_t k = rowfin[a];
							attacked[k] = 1;
							decap_xp.attacked.push_back(cand[fin[k]]);
							float sv = 0.0f, mn = 1e30f, wsum = 0.0f;
							for (const auto &x : cv[a]) sv += x.second * x.first, wsum += x.second, mn = std::min(mn, x.first);
							float d = 0.0f;
							if (wsum > 0.0f) {
								float mean2 = cs_eps > 0.0f ? (1.0f - cs_eps) * fmean[k] + cs_eps * (sv / wsum) : (fcnt[k] * fmean[k] + sv) / (fcnt[k] + wsum);
								d = (1.0f - xw_worst) * (mean2 - fmean[k]);
								if (cs_mode == 1 && mn < fmin[k]) d -= xw_worst * (fmin[k] - mn);
								if (cs_mode == 2 && std::isfinite(q0[k])) {
									std::vector<std::pair<float, float>> vw = reply_vals(k);
									vw.insert(vw.end(), cv[a].begin(), cv[a].end());
									float q1 = softq(vw);
									if (std::isfinite(q1)) d += xw_worst * (q1 - q0[k]);
								}
							}
							if (cs_cap > 0.0f) d = std::min(cs_cap, std::max(-cs_cap, d));
							delta[k] = d;
							if (cs_veto && cu[a]) vetoed[k] = 1;
							{
								char line[400];
								// [game access]
								for (const auto &x : cv[a]) if (len < 330) len += snprintf(line + len, sizeof(line) - len, " %.3f", x.first);
								// [game access]
								decap_cs_log(line);
							}
						}
						if (cs_target != 0) break;
					}
					decap_xp.on = xon;
				}

				int n_att = 0;
				float dsum = 0.0f;
				for (size_t k = 0; k < nf; k++) if (attacked[k]) n_att++, dsum += delta[k];
				float mean_delta = n_att ? dsum / n_att : 0.0f;
				if (n_att) {
					for (size_t k = 0; k < nf; k++) if (!attacked[k]) delta[k] = mean_delta;
				}
				if (status == 1) {
					for (size_t k = 0; k < nf; k++) fscore[k] = base[k] + delta[k];
				}
				if (cs_veto) {
					int left_safe = 0;
					for (size_t k = 0; k < nf; k++) left_safe += fsafe[k] && !vetoed[k];
					if (left_safe > 0) for (size_t k = 0; k < nf; k++) if (vetoed[k]) fsafe[k] = 0;
				}
				int kb1 = -1;
				for (size_t k = 0; k < nf; k++) {
					if (fsafe[k] && (kb1 < 0 || fscore[k] > fscore[kb1])) kb1 = (int)k;
				}
				if (status == 1 && (cut || answered < full_plans)) status = 5;

				if (status == 1 || status == 5) cem_note |= 8;
				if (status == 5) cem_note |= 16;
				decap_cs.status = (int16_t)status;
				decap_cs.rounds = (int16_t)rounds;
				decap_cs.attacked = (uint16_t)n_att;
				decap_cs.turn_sims = (uint16_t)std::min(turn_sims, 65535);
				decap_cs.full_plans = (uint16_t)std::min(full_plans, 65535);
				decap_cs.answered = (uint16_t)std::min(answered, 65535);
				decap_cs.kept = (uint16_t)std::min(kept_n, 65535);
				decap_cs.changed = (uint16_t)(kb0 != kb1);
				decap_cs.ms_used = (int32_t)(decap_ms_since(t0) - decap_cs.ms_start);
				decap_cs.guess_taken = guess_taken;
				decap_cs.counter_taken = counter_taken;
				decap_cs.guess_j = guess_j;
				decap_cs.counter_j = counter_j;
				decap_cs.fs_before = kb0 >= 0 ? base[kb0] : NAN;
				decap_cs.fs_after = kb0 >= 0 ? fscore[kb0] : NAN;
				decap_cs.mean_delta = mean_delta;
				if (decap_env_int("DECAP_LOOKAHEAD_LOG", 0)) {
					// [game access: 4 lines]
				}
			}

			decap_lf_snap(3, fscore, fsafe, fmin);
			const float pick1_w = decap_env_int("DECAP_PICK1_MILLI", 0) / 1000.0f;
			if (pick1_w > 0.0f && decap_xp.on && !rescue_cem && fin.size() > 1 && first.d0 < decap_env_int("DECAP_PICK1_NEAR_MILLI", 2500) / 1000.0f) {
				std::vector<float> one(fin.size(), NAN);
				float lowest = NAN;
				for (size_t k = 0; k < fin.size(); k++) {
					int slot = -1;
					for (size_t s = 0; s < decap_xp.cand.size() && slot < 0; s++) {
						if (decap_xp_same(decap_xp.cand[s].states, decap_xp.cand[s].grip, cand[fin[k]])) slot = (int)s;
					}
					if (slot < 0) continue;
					double sum = 0.0;
					int n = 0;
					bool danger = false;
					for (const decap_xp_out &x : decap_xp.outs[slot]) {
						if (!(x.flags & 1)) continue;
						if ((x.flags & 2) || ((x.flags & (4 | 8)) && !(x.flags & 16))) danger = true;
						if (x.rep >= decap_xp.reps.size() || decap_xp.reps[x.rep].src == 7) continue;
						if (!std::isfinite(x.dealt1) || !std::isfinite(x.taken1)) continue;
						sum += x.dealt1 - x.taken1;
						n++;
					}
					if (n > 0 && !danger) {
						one[k] = race_dec ? decap_race_one(cand[fin[k]], (float)(sum / n)) : (float)(sum / n);
						if (!std::isfinite(lowest) || one[k] < lowest) lowest = one[k];
					}
				}
				if (std::isfinite(lowest)) {
					int kb = -1, ka = -1;
					for (size_t k = 0; k < fin.size(); k++) if (fsafe[k] && (kb < 0 || fscore[k] > fscore[kb])) kb = (int)k;
					for (size_t k = 0; k < fin.size(); k++) fscore[k] += pick1_w * (std::isfinite(one[k]) ? one[k] : lowest) / 20000.0f;
					for (size_t k = 0; k < fin.size(); k++) if (fsafe[k] && (ka < 0 || fscore[k] > fscore[ka])) ka = (int)k;
					cem_note |= 32;
					if (decap_env_int("DECAP_LOOKAHEAD_LOG", 0) && kb >= 0 && ka >= 0) {
						// [game access: 2 lines]
					}
				}
			}

			decap_lf_snap(4, fscore, fsafe, fmin);
			const float lp_play = decap_env_int("DECAP_LP_PLAY_MILLI", 0) / 1000.0f;
			int lp_pk = -1, lp_pn = 0, lp_ps = 0;
			float lp_pk_before = NAN, lp_pk_other = NAN;
			if (!(lp_play > 0.0f && lp_cn > 0 && !rescue_cem)) cem_note &= ~512;
			if (lp_play > 0.0f && lp_cn > 0 && !rescue_cem && lp_bonus_ok) {
				auto fs = [&](size_t k) { return std::isfinite(fscore[k]) ? fscore[k] : -1e9f; };
				int kb = -1, ka = -1, ko = -1;
				const bool rg_lp_on = decap_rg_on();
				const float rg_lp = decap_env_int("DECAP_RG_LP_MILLI", 200) / 1000.0f;
				for (size_t k = 0; k < fin.size(); k++) {
					if (!lp_is(fin[k]) || fin[k] - lp_c0 >= lp_kj.size()) continue;
					lp_pn++;
					if (!fsafe[k]) continue;
					lp_ps++;
					if (rg_lp_on && decap_rg_mean(cand[fin[k]]) >= rg_lp) {
						cem_note |= 1024;
						continue;
					}
					if (lp_pk < 0 || lp_kj[fin[k] - lp_c0] > lp_kj[fin[lp_pk] - lp_c0]) lp_pk = (int)k;
				}
				if (lp_pk >= 0) {
					for (size_t k = 0; k < fin.size(); k++) {
						if (!fsafe[k]) continue;
						if (kb < 0 || fs(k) > fs(kb)) kb = (int)k;
						if ((int)k != lp_pk && (ko < 0 || fs(k) > fs(ko))) ko = (int)k;
					}
					lp_pk_before = fscore[lp_pk];
					lp_pk_other = ko >= 0 ? fscore[ko] : NAN;
					fscore[lp_pk] += lp_play;
					for (size_t k = 0; k < fin.size(); k++) if (fsafe[k] && (ka < 0 || fs(k) > fs(ka))) ka = (int)k;
					if (ka != kb) cem_note |= 128;
				}
			}
			int played_c = -1;
			double rg_u = -1.0;
			if (!fin.empty()) {
				float best = 0.0f;
				int k = 0;
				if (!rescue_cem) {
					rg_u = (double)rand() / ((double)RAND_MAX + 1.0);
					k = bot::pickCandidate(fscore, fsafe, tau, rg_u, &best);
				}
				if (k >= 0) {
					chosen = cand[fin[k]];
					played_c = (int)fin[k];
					cem_note |= fin[k] >= mine.size() && !lp_is(fin[k]) ? 1 : 0;
					pick.score = stage_score(fin[k]);
				}
			}

			if (decap_rg_on() && decap_env_int("DECAP_RG_LOG", 0) && !fin.empty() && played_c >= 0) {
				const float w = decap_rg_weight(seat);
				auto fs0 = [&](size_t k) { return std::isfinite(fscore[k]) ? fscore[k] : -1e9f; };
				std::vector<float> ns(fin.size());
				for (size_t k = 0; k < fin.size(); k++) ns[k] = fs0(k) + w * 0.5f * (decap_rg_mean(cand[fin[k]]) + decap_rg_max(cand[fin[k]]));
				if (lp_play > 0.0f && lp_cn > 0 && !rescue_cem && lp_bonus_ok) {
					int lp0 = -1;
					for (size_t k = 0; k < fin.size(); k++) {
						if (!lp_is(fin[k]) || fin[k] - lp_c0 >= lp_kj.size() || !fsafe[k]) continue;
						if (lp0 < 0 || lp_kj[fin[k] - lp_c0] > lp_kj[fin[lp0] - lp_c0]) lp0 = (int)k;
					}
					if (lp0 != lp_pk) {
						if (lp_pk >= 0) ns[lp_pk] -= lp_play;
						if (lp0 >= 0) ns[lp0] += lp_play;
					}
				}
				int kn = -1, kp = -1;
				for (size_t k = 0; k < fin.size(); k++) {
					if ((int)fin[k] == played_c) kp = (int)k;
				}
				if (!rescue_cem && rg_u >= 0.0) {
					float b2 = 0.0f;
					kn = bot::pickCandidate(ns, fsafe, tau, rg_u, &b2);
				}
				const bool changed = kn >= 0 && kp >= 0 && kn != kp;
				const int kq = changed ? kn : kp;
				float anymax = 0.0f;
				for (const auto &kv : decap_rg_seen) if (kv.second.n > 0) anymax = std::max(anymax, (float)(kv.second.sum / kv.second.n));
				char jp[N_JOINTS + 1], jn[N_JOINTS + 1];
				for (int j = 0; j < N_JOINTS; j++) {
					jp[j] = (char)('0' + chosen.states[j]);
					jn[j] = (char)('0' + (kq >= 0 ? cand[fin[kq]].states[j] : chosen.states[j]));
				}
				jp[N_JOINTS] = jn[N_JOINTS] = 0;
				char line[500];
				// [game access: 4 lines]
				const char *dir = getenv("DECAP_DECISION_LOG");
				if (dir && *dir) {
					char path[600];
					// [game access]
					int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
					if (fd >= 0) {
						ssize_t wr = write(fd, line, strlen(line));
						(void)wr;
						close(fd);
					}
				}
			}
			if (lp_play > 0.0f && lp_cn > 0 && !rescue_cem) {
				const bool lp_log = decap_env_int("DECAP_LOOKAHEAD_LOG", 0) != 0, lp_file = decap_env_int("DECAP_LP_LOG", 0) != 0;
				if (lp_log || lp_file) {
					char line[400];
					// [game access: 4 lines]
					if (!lp_bonus_ok) {
						size_t L = strlen(line);
						if (L > 0 && line[L - 1] == 10) line[--L] = 0;
						// [game access]
					}
					// [game access]
					const char *dir = getenv("DECAP_DECISION_LOG");
					if (lp_file && dir && *dir) {
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
			}
#include "decap_ko.inc"
#include "decap_entry.inc"
// [game access]
			if (decap_rg_on()) {
				for (const auto &kv : decap_rg_seen) {
					if (kv.second.n > 0 && kv.second.sum / kv.second.n >= 0.15) {
						cem_note |= 2048;
						break;
					}
				}
				if (decap_rg_mean(chosen) >= 0.05f) cem_note |= 16384;
			}
			decap_diag = first;
			DECAP_LF_FINAL();
			decap_diag.played = ((cem_note & 1) || (played_c >= 0 && lp_is((size_t)played_c))) ? -1 : pick.candidate;
			if (ko_changed) decap_diag.played = (played_c >= 0 && (size_t)played_c < mine.size()) ? played_c : -1;
			decap_publish(chosen, pick.score, 3);
			{
				auto as_cand = [&](size_t c) {
					decap_dec_cand d;
					memcpy(d.joints, cand[c].states, N_JOINTS);
					d.grip[0] = cand[c].grip[0];
					d.grip[1] = cand[c].grip[1];
					d.safe = rescue_cem ? c_mean[c] >= 1.0f : c_safe[c];
					d.tag = lp_is(c) ? 13 : (c < mine.size() && c < DECAP_DIAG_MAX ? first.tag[c] : 8);
					d.mean_v = c_mean[c];
					d.min_v = c_min[c];
					d.fscore = NAN;
					return d;
				};
				decap_last_dec.cands.clear();
				for (size_t c = 0; c < cand.size() && c < DECAP_DIAG_MAX; c++) decap_last_dec.cands.push_back(as_cand(c));
				for (size_t k = 0; k < fin.size(); k++) {
					if (fin[k] < decap_last_dec.cands.size()) decap_last_dec.cands[fin[k]].fscore = fscore[k];
				}
				decap_last_dec.played_idx = played_c >= 0 ? played_c : pick.candidate;
				if (played_c >= DECAP_DIAG_MAX) {
					decap_last_dec.cands.back() = as_cand((size_t)played_c);
					for (size_t k = 0; k < fin.size(); k++) {
						if ((int)fin[k] == played_c) decap_last_dec.cands.back().fscore = fscore[k];
					}
					decap_last_dec.played_idx = (int32_t)decap_last_dec.cands.size() - 1;
				}
				decap_last_dec.played = chosen;
				decap_last_dec.cem_note = cem_note;
				decap_last_dec.score = pick.score;
				decap_last_dec.valid = true;
			}
		}
		if (decap_env_int("DECAP_LOOKAHEAD_LOG", 0)) {
			int knn_moves = 0, played_knn = 0;
			for (size_t c = 0; c < mine.size(); c++) {
				bool from_knn = false;
				for (const Move &r : similar) {
					from_knn |= memcmp(r.states, mine[c].states, sizeof(r.states)) == 0;
				}
				knn_moves += from_knn;
				played_knn |= from_knn && (int)c == pick.candidate;
			}
			// [game access: 7 lines]
		}
// [game access]
	}
	else
#endif
	{
		// [game access]
	}
	return chosen;
}

void decap_save_turn_joints()
{
	for (int i = 0; i < N_FIGHTERS; i++) {
		for (int j = 0; j < N_JOINTS; j++) {
			// [game access: 3 lines]
		}
		// [game access]
	}
