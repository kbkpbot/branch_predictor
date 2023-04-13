module main

import time
import os
import cbsl { CBSL_ERRORS, CBSL_MODE, cbsl_close, cbsl_open, cbsl_read }

const (
	min_miss_per_1k = 30 // use to score the result
	max_miss_per_1k = 100 // use to score the result
	taken           = u8(0x54)
	not_taken       = u8(0x4E)
	max_spawn_num   = 1 // num of threads spawn to accelerate the simulaion
)

enum OpType {
	optype_op = 2
	optype_ret_uncond
	optype_jmp_direct_uncond
	optype_jmp_indirect_uncond
	optype_call_direct_uncond
	optype_call_indirect_uncond
	optype_ret_cond
	optype_jmp_direct_cond
	optype_jmp_indirect_cond
	optype_call_direct_cond
	optype_call_indirect_cond
	optype_error
	optype_max
}

struct BT9_NODE {
	virtual_address  u64
	physical_address u64
	opcode           u64
	optype           u32
	size             u32
}

// EDGE id can't exceed 2^32
struct BT9_EDGE {
	src_id         u32
	dest_id        u32
	taken          u8
	br_virt_target u64
	br_phy_target  u64
	inst_cnt       u64
	traverse_cnt   u64
}

struct BT9_struct {
mut:
	bt9_minor_version                    u32
	has_physical_address                 u32
	md5_checksum                         u32
	conversion_date                      u32
	original_stf_input_file              string
	total_instruction_count              u64
	branch_instruction_count             u64
	invalid_physical_branch_target_count u64
	a32_instruction_count                u64
	a64_instruction_count                u64
	t32_instruction_count                u64
	unidentified_instruction_count       u64
	bt9_node_count                       u32
	bt9_edge_count                       u32
	bt9_trace_count                      u32
	node                                 []BT9_NODE
	edge                                 []BT9_EDGE
	trace                                []u32
}

fn cbsl_error_check(msg CBSL_ERRORS) {
	if msg != CBSL_ERRORS.cbsl_success && msg != CBSL_ERRORS.cbsl_end {
		println('cbsl error: ' + msg.str())
	}
}

// load_bt9 load a bt9 bin file into `BT9_struct`.
fn load_bt9(filename string) !BT9_struct {
	println('[${@FN}] Reading trace file[${filename}], please wait...')
	mut ctx := unsafe { nil }
	ctx = cbsl_open(CBSL_MODE.cbsl_load_mode, filename)
	if isnil(ctx) {
		return error('Can\'t open the ${filename} for reading.')
	}
	mut bt9 := BT9_struct{}
	mut original_stf_input_file := []u8{len: 256}
	cbsl_error_check(cbsl_read(ctx, &bt9.bt9_minor_version, sizeof(u32)))
	cbsl_error_check(cbsl_read(ctx, &bt9.has_physical_address, sizeof(u32)))
	cbsl_error_check(cbsl_read(ctx, &bt9.md5_checksum, sizeof(u32)))
	cbsl_error_check(cbsl_read(ctx, &bt9.conversion_date, sizeof(u32)))
	cbsl_error_check(cbsl_read(ctx, original_stf_input_file.data, 256))
	bt9.original_stf_input_file = filename
	cbsl_error_check(cbsl_read(ctx, &bt9.total_instruction_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.branch_instruction_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.invalid_physical_branch_target_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.a32_instruction_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.a64_instruction_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.t32_instruction_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.unidentified_instruction_count, sizeof(u64)))
	cbsl_error_check(cbsl_read(ctx, &bt9.bt9_node_count, sizeof(u32)))
	cbsl_error_check(cbsl_read(ctx, &bt9.bt9_edge_count, sizeof(u32)))
	cbsl_error_check(cbsl_read(ctx, &bt9.bt9_trace_count, sizeof(u32)))
	// skip struct padding
	mut tmp := []u8{len: 28}
	cbsl_error_check(cbsl_read(ctx, tmp.data, 28 * sizeof(u8)))

	mut node := []BT9_NODE{len: int(bt9.bt9_node_count)}
	mut edge := []BT9_EDGE{len: int(bt9.bt9_edge_count)}
	mut trace := []u32{len: int(bt9.bt9_trace_count)}
	cbsl_error_check(cbsl_read(ctx, node.data, u64(bt9.bt9_node_count * sizeof(BT9_NODE))))
	cbsl_error_check(cbsl_read(ctx, edge.data, u64(bt9.bt9_edge_count * sizeof(BT9_EDGE))))
	cbsl_error_check(cbsl_read(ctx, trace.data, u64(bt9.bt9_trace_count * sizeof(u32))))
	cbsl_error_check(cbsl_close(ctx))

	bt9.node = node
	bt9.edge = edge
	bt9.trace = trace
	return bt9
}

// sim_bt9 simute the `bt9`, it will read traces from `bt9` and call user predictor functions.
fn sim_bt9(bt9 BT9_struct) f64 {
	println('[${@FN}] Begin the branch prediction simulation, please wait...')
	mut cond_br_count := u64(0)
	mut miss_count := u64(0)
	mut uncond_br_count := u64(0)
	mut optype := OpType.optype_error
	mut ps := Predictor_Struct{}
	ps.predictor_init()

	// skip the first record, it is an invalid record
	for i in 1 .. bt9.bt9_trace_count {
		edge_id := bt9.trace[i]
		edge := bt9.edge[edge_id]
		src_node := bt9.node[edge.src_id]
		pc := src_node.virtual_address
		resolvedir := edge.taken
		branchtarget := edge.br_virt_target
		unsafe {
			optype = OpType(src_node.optype)
		}
		// only handle the condition jumps. Note: It seems the test sets only contain OPTYPE_JMP_DIRECT_CONDDIRECT_COND
		if optype == .optype_ret_cond || optype == .optype_jmp_direct_cond
			|| optype == .optype_jmp_indirect_cond || optype == .optype_call_direct_cond
			|| optype == .optype_call_indirect_cond {
			cond_br_count++
			preddir := ps.get_prediction(pc)
			ps.update_predictor(pc, optype, resolvedir, preddir, branchtarget)
			if resolvedir != preddir {
				miss_count++
			}
		} else {
			uncond_br_count++
		}
	}
	ps.predictor_free()

	mispred_per_1k_inst := 1000.0 * f64(miss_count) / f64(bt9.total_instruction_count)
	println('  NUM_INSTRUCTIONS            \t : ${bt9.total_instruction_count:10u}')
	println('  NUM_BR                      \t : ${bt9.branch_instruction_count - 1:10u}')
	println('  NUM_UNCOND_BR               \t : ${uncond_br_count:10u}')
	println('  NUM_CONDITIONAL_BR          \t : ${cond_br_count:10u}')
	println('  NUM_MISPREDICTIONS          \t : ${miss_count:10u}')
	println('  MISPRED_PER_1K_INST         \t : ${mispred_per_1k_inst:10.4f}')
	return mispred_per_1k_inst
}

struct SimResult {
mut:
	filename            string
	mispred_per_1k_inst f64
}

// sim_filenames simulate a list of filenames.
fn sim_filenames(filenames []string) []SimResult {
	mut sim_result := []SimResult{}
	for filename in filenames {
		bt9 := load_bt9(filename) or { BT9_struct{} }
		sim_result << SimResult{
			filename: filename
			mispred_per_1k_inst: sim_bt9(bt9)
		}
	}

	return sim_result
}

fn main() {
	mut total_miss_per_1k := f64(0)
	// Try to use multi threads, but it seems not work
	mut spawn_filenames := [][]string{len: max_spawn_num}
	mut i := int(0)

	tick1 := time.ticks()

	if os.args.len == 1 {
		// we will reading the default TRACE_LIST.txt
		file_lists := os.read_lines('traces/TRACE_LIST.txt')!
		for line in file_lists {
			if line.starts_with('#') {
				continue
			} else {
				filename := 'traces/' + line.trim_space()
				spawn_filenames[i % max_spawn_num] << filename
			}
		}

		mut threads := []thread []SimResult{}
		for j in 0 .. max_spawn_num {
			threads << spawn sim_filenames(spawn_filenames[j])
		}
		mut r := threads.wait()
		// print(r)
		r[0].sort(b.filename > a.filename)
		t := r[0]
		// println(t)
		println('====================Summary====================')
		for f in t {
			println('${f.filename}\t\t= ${f.mispred_per_1k_inst:.6f}')
			total_miss_per_1k += f.mispred_per_1k_inst
		}
		println('===============================================')
		println('Total MISPRED_PER_1K_INST\t\t= ${total_miss_per_1k:.1f}')
		if total_miss_per_1k > max_miss_per_1k {
			total_miss_per_1k = max_miss_per_1k
		}
		if total_miss_per_1k < min_miss_per_1k {
			total_miss_per_1k = min_miss_per_1k
		}

		score := int(((max_miss_per_1k - total_miss_per_1k) / (max_miss_per_1k - min_miss_per_1k)) * 100.0)
		tick2 := time.ticks()
		time_cost := f32((tick2 - tick1)) / 1000.0
		println('\nScore = ${score}(${total_miss_per_1k:.1f}) time cost = ${time_cost} seconds')
		return
	} else {
		// we will simulate a specified bt9.
		bt9 := load_bt9(os.args[1])!
		println('filename = ${bt9.original_stf_input_file}')
		println('bt9_node_count = ${bt9.bt9_node_count}')
		println('bt9_edge_count = ${bt9.bt9_edge_count}')
		println('bt9_trace_count = ${bt9.bt9_trace_count}')
		sim_bt9(bt9)
	}
}
