module main

const (
	pht_ctr_max  = 3 // sat counter max value
	pht_ctr_init = 3 // sat counter init value
	hist_len     = 17 // global history register's length in bits
)

// sat_increment increase the sat counter.
fn sat_increment(x u8) u8 {
	return if x < pht_ctr_max { x + 1 } else { x }
}

// sat_decrement decrease the sat counter.
fn sat_decrement(x u8) u8 {
	return if x > 0 { x - 1 } else { x }
}

// Predictor_Struct save the predictor state.
struct Predictor_Struct {
mut: // The state is defined for Gshare, change for your design
	ghr             u32  // global history register
	pht             []u8 = []u8{len: 1 << hist_len, init: pht_ctr_init} // pattern history table
	history_length  u32  = hist_len // history length
	num_pht_entries u32  = (1 << hist_len) // entries in pht
}

// predictor_init init the predictor, currently it does nothing.
fn (mut ps Predictor_Struct) predictor_init() {
}

// predictor_free free the predictor, currently it does nothing.
fn (mut ps Predictor_Struct) predictor_free() {
}

// get_prediction get the prediction from current `pc`, return taken or not_taken.
fn (ps Predictor_Struct) get_prediction(pc u64) u8 {
	pht_index := (pc ^ ps.ghr) % ps.num_pht_entries
	pht_counter := ps.pht[pht_index]

	if pht_counter > (pht_ctr_max / 2) {
		return taken
	} else {
		return not_taken
	}
}

// update_predictor update the predictor state, according to `resolve_dir` and `pred_dir`.
fn (mut ps Predictor_Struct) update_predictor(pc u64, optype OpType, resolve_dir u8, pred_dir u8, branch_target u64) {
	pht_index := (pc ^ ps.ghr) % ps.num_pht_entries
	pht_counter := ps.pht[pht_index]

	if resolve_dir == taken {
		ps.pht[pht_index] = sat_increment(pht_counter) // if result is taken, then increase the sat counter
	} else {
		ps.pht[pht_index] = sat_decrement(pht_counter) // if result is not_taken, then decrease the sat counter
	}

	// update the ghr
	ps.ghr = (ps.ghr << 1)

	if resolve_dir == taken {
		ps.ghr = ps.ghr | 0x1
	}
}
