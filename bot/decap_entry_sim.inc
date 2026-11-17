// decapAI bot, build B53: support for the entry planner: running two-turn plans, and the per-game switch for comparing with and without it.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

struct decap_en_out {
	int32_t ok;
	uint8_t lost1, end1;
	uint8_t lost2, pad;
	float dealt1, taken1;
	float dealt2, taken2;
};

#pragma pack(push, 1)
struct decap_en_lf_t {
	uint16_t hdr_size, version;
	uint8_t status;
	uint8_t decision;

	uint8_t fail_sets;
	uint8_t why[3];
	uint8_t n_ans[3], n_set[3];
	float d0, lead;
	int32_t pop, gens_cfg, gens_done, elites, n_rep, n_seed, n_seq, n_plan, n_turn, noans, disp, disp_fail;
	int32_t ms_start, ms_cem, ms_safe, ms_stage, ms_cem_end, ms_safe_end;
	float J_best, J_NN, J_Nbest, J_F_N2;
	float n_score, f_score;
	float J_gen[8];
	uint8_t N[N_JOINTS], F[N_JOINTS], F2[N_JOINTS];
	int32_t f_cand;
	int32_t ms_gen[8];

	int32_t t1_hits;
	int32_t gen_cut;
	int32_t partial_seqs;

	int32_t min_gens;
};
#pragma pack(pop)
struct decap_en_rec_t {
	bool have;
	decap_en_lf_t h;
};
static decap_en_rec_t decap_en_lf;

static void decap_en_turn(int me, const Move &mb, const Move &r)
{
	// [REDACTED: this function mainly reads the game's state; 4 lines]
}
static void decap_en_group(int me, const Move &m1, const Move &r, const std::vector<const Move *> &m2s,
                           const std::vector<size_t> &slots, decap_en_out *shm)
{
	// [REDACTED: this function mainly reads the game's state; 36 lines]
}

#if BOT_BUILD
// [game access]
#endif
static int64_t decap_ab_svc_value = -1;
static uint64_t decap_ab_mix(uint64_t x)
{
	// [REDACTED: this function mainly reads the game's state; 4 lines]
}
static int64_t decap_ab_value()
{
	// [REDACTED: this function mainly reads the game's state; 5 lines]
}
static int decap_ab_coin_of(int64_t v) { return (int)((decap_ab_mix((uint64_t)v) >> 32) & 1u); }

static int decap_ab_coin()
{
	// [game access]
	return decap_ab_coin_of(decap_ab_value());
}

static void decap_ab_say()
{
	// [REDACTED: this function mainly reads the game's state; 9 lines]
}
