/**
 * vim: set ts=8 sw=2 tw=99 sts=2 et:
 * =============================================================================
 * SourceMod
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License), version 3.0), as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful), but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not), see <http://www.gnu.org/licenses/>.
 *
 * As a special exception), AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2)," the
 * "Source Engine)," the "SourcePawn JIT)," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally), AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions), found in LICENSE.txt (as of this writing), version JULY-31-2007)),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEPAWN_JIT_X86_OPCODES_H_
#define _INCLUDE_SOURCEPAWN_JIT_X86_OPCODES_H_

#include <sp_vm_api.h>

// Opcodes labelled "UNGEN" cannot be generated by the compiler. Quite a few,
// if they could, make little sense in the context of a JIT and could not
// work anyway. Opcodes technically present in sc4.c/sc7.c (respectively,
// the code emitter and peephole optimizer) are not necessarily ever generated.
// For example, lref.pri and lref.alt would be used if a reference could be
// stored in the global scope; but they can't, so they are unreachable.
//
// Most opcodes have been manually verified. A few have not, as they are only
// produced by the peephole optimizer, or not produced at all, or eliminated
// during optimization. We generate them anyway, just in case, but they have
// not been tested.
//    lref.s.alt (phopt only)
//    stor.alt (never)
//    stor.s.alt (never)
//    sref.s.alt (never)
//    lidx.b (phopt only, probably impossible)
//    idxaddr.b (phopt only, looks difficult)
//    move.pri (eliminated in phopt)
//    shl.c.pri (eliminated in phopt)
//    shl.c.alt (eliminated in phopt)
//    shr.c.pri (eliminated in phopt)
//    shr.c.alt (eliminated in phopt)
//    eq.c.alt (never)
//    inc.alt (never)
//    dec.alt (never)
//    sdiv (never)
//    nop (never in function bodies)
//
// Additionally, some opcodes which were supported in the earlier JIT are no
// longer supported because of the above:
//    lref.pri/alt
//    sref.pri/alt
//    sign.pri/alt

#define OPCODE_LIST(_)                \
  _(NONE,           "none")           \
  _(LOAD_PRI,       "load.pri")       \
  _(LOAD_ALT,       "load.alt")       \
  _(LOAD_S_PRI,     "load.s.pri")     \
  _(LOAD_S_ALT,     "load.s.alt")     \
  _(UNGEN_LREF_PRI, "lref.pri")       \
  _(UNGEN_LREF_ALT, "lref.alt")       \
  _(LREF_S_PRI,     "lref.s.pri")     \
  _(LREF_S_ALT,     "lref.s.alt")     \
  _(LOAD_I,         "load.i")         \
  _(LODB_I,         "lodb.i")         \
  _(CONST_PRI,      "const.pri")      \
  _(CONST_ALT,      "const.alt")      \
  _(ADDR_PRI,       "addr.pri")       \
  _(ADDR_ALT,       "addr.alt")       \
  _(STOR_PRI,       "stor.pri")       \
  _(STOR_ALT,       "stor.alt")       \
  _(STOR_S_PRI,     "stor.s.pri")     \
  _(STOR_S_ALT,     "stor.s.alt")     \
  _(UNGEN_SREF_PRI, "sref.pri")       \
  _(UNGEN_SREF_ALT, "sref.alt")       \
  _(SREF_S_PRI,     "sref.s.pri")     \
  _(SREF_S_ALT,     "sref.s.alt")     \
  _(STOR_I,         "stor.i")         \
  _(STRB_I,         "strb.i")         \
  _(LIDX,           "lidx")           \
  _(LIDX_B,         "lidx.b")         \
  _(IDXADDR,        "idxaddr")        \
  _(IDXADDR_B,      "idxaddr.b")      \
  _(UNGEN_ALIGN_PRI,"align.pri")      \
  _(UNGEN_ALIGN_ALT,"align.alt")      \
  _(UNGEN_LCTRL,    "lctrl")          \
  _(UNGEN_SCTRL,    "sctrl")          \
  _(MOVE_PRI,       "move.pri")       \
  _(MOVE_ALT,       "move.alt")       \
  _(XCHG,           "xchg")           \
  _(PUSH_PRI,       "push.pri")       \
  _(PUSH_ALT,       "push.alt")       \
  _(UNGEN_PUSH_R,   "push.r")         \
  _(PUSH_C,         "push.c")         \
  _(PUSH,           "push")           \
  _(PUSH_S,         "push.s")         \
  _(POP_PRI,        "pop.pri")        \
  _(POP_ALT,        "pop.alt")        \
  _(STACK,          "stack")          \
  _(HEAP,           "heap")           \
  _(PROC,           "proc")           \
  _(UNGEN_RET,      "ret")            \
  _(RETN,           "retn")           \
  _(CALL,           "call")           \
  _(UNGEN_CALL_PRI, "call.pri")       \
  _(JUMP,           "jump")           \
  _(UNGEN_JREL,     "jrel")           \
  _(JZER,           "jzer")           \
  _(JNZ,            "jnz")            \
  _(JEQ,            "jeq")            \
  _(JNEQ,           "jneq")           \
  _(UNGEN_JLESS,    "jsless")         \
  _(UNGEN_JLEQ,     "jleq")           \
  _(UNGEN_JGRTR,    "jgrtr")          \
  _(UNGEN_JGEQ,     "jgeq")           \
  _(JSLESS,         "jsless")         \
  _(JSLEQ,          "jsleq")          \
  _(JSGRTR,         "jsgrtr")         \
  _(JSGEQ,          "jsgeq")          \
  _(SHL,            "shl")            \
  _(SHR,            "shr")            \
  _(SSHR,           "sshr")           \
  _(SHL_C_PRI,      "shl.c.pri")      \
  _(SHL_C_ALT,      "shl.c.alt")      \
  _(SHR_C_PRI,      "shr.c.pri")      \
  _(SHR_C_ALT,      "shr.c.alt")      \
  _(SMUL,           "smul")           \
  _(SDIV,           "sdiv")           \
  _(SDIV_ALT,       "sdiv.alt")       \
  _(UNGEN_UMUL,     "umul")           \
  _(UNGEN_UDIV,     "udiv")           \
  _(UNGEN_UDIV_ALT, "udiv.alt")       \
  _(ADD,            "add")            \
  _(SUB,            "sub")            \
  _(SUB_ALT,        "sub.alt")        \
  _(AND,            "and")            \
  _(OR,             "or")             \
  _(XOR,            "xor")            \
  _(NOT,            "not")            \
  _(NEG,            "neg")            \
  _(INVERT,         "invert")         \
  _(ADD_C,          "add.c")          \
  _(SMUL_C,         "smul.c")         \
  _(ZERO_PRI,       "zero.pri")       \
  _(ZERO_ALT,       "zero.alt")       \
  _(ZERO,           "zero")           \
  _(ZERO_S,         "zero.s")         \
  _(UNGEN_SIGN_PRI, "sign.pri")       \
  _(UNGEN_SIGN_ALT, "sign.alt")       \
  _(EQ,             "eq")             \
  _(NEQ,            "neq")            \
  _(UNGEN_LESS,     "less")           \
  _(UNGEN_LEQ,      "leq")            \
  _(UNGEN_GRTR,     "grtr")           \
  _(UNGEN_GEQ,      "geq")            \
  _(SLESS,          "sless")          \
  _(SLEQ,           "sleq")           \
  _(SGRTR,          "sgrtr")          \
  _(SGEQ,           "sgeq")           \
  _(EQ_C_PRI,       "eq.c.pri")       \
  _(EQ_C_ALT,       "eq.c.alt")       \
  _(INC_PRI,        "inc.pri")        \
  _(INC_ALT,        "inc.alt")        \
  _(INC,            "inc")            \
  _(INC_S,          "inc.s")          \
  _(INC_I,          "inc.i")          \
  _(DEC_PRI,        "dec.pri")        \
  _(DEC_ALT,        "dec.alt")        \
  _(DEC,            "dec")            \
  _(DEC_S,          "dec.s")          \
  _(DEC_I,          "dec.i")          \
  _(MOVS,           "movs")           \
  _(UNGEN_CMPS,     "cmps")           \
  _(FILL,           "fill")           \
  _(HALT,           "halt")           \
  _(BOUNDS,         "bounds")         \
  _(UNGEN_SYSREQ_PRI,"sysreq.pri")     \
  _(SYSREQ_C,       "sysreq.c")       \
  _(UNGEN_FILE,     "file")           \
  _(UNGEN_LINE,     "line")           \
  _(UNGEN_SYMBOL,   "symbol")         \
  _(UNGEN_SRANGE,   "srange")         \
  _(UNGEN_JUMP_PRI, "jump.pri")       \
  _(SWITCH,         "switch")         \
  _(CASETBL,        "casetbl")        \
  _(SWAP_PRI,       "swap.pri")       \
  _(SWAP_ALT,       "swap.alt")       \
  _(PUSH_ADR,       "push.adr")       \
  _(NOP,            "nop")            \
  _(SYSREQ_N,       "sysreq.n")       \
  _(UNGEN_SYMTAG,   "symtag")         \
  _(BREAK,          "break")          \
  _(PUSH2_C,        "push2.c")        \
  _(PUSH2,          "push2")          \
  _(PUSH2_S,        "push2.s")        \
  _(PUSH2_ADR,      "push2.adr")      \
  _(PUSH3_C,        "push3.c")        \
  _(PUSH3,          "push3")          \
  _(PUSH3_S,        "push3.s")        \
  _(PUSH3_ADR,      "push3.adr")      \
  _(PUSH4_C,        "push4.c")        \
  _(PUSH4,          "push4")          \
  _(PUSH4_S,        "push4.s")        \
  _(PUSH4_ADR,      "push4.adr")      \
  _(PUSH5_C,        "push5.c")        \
  _(PUSH5,          "push5")          \
  _(PUSH5_S,        "push5.s")        \
  _(PUSH5_ADR,      "push5.adr")      \
  _(LOAD_BOTH,      "load.both")      \
  _(LOAD_S_BOTH,    "load.s.both")    \
  _(CONST,          "const")          \
  _(CONST_S,        "const.s")        \
  _(UNGEN_SYSREQ_D, "sysreq.d")       \
  _(UNGEB_SYSREQ_ND,"sysreq.nd")      \
  _(TRACKER_PUSH_C, "trk.push.c")     \
  _(TRACKER_POP_SETHEAP,"trk.pop")   \
  _(GENARRAY,       "genarray")       \
  _(GENARRAY_Z,     "genarray.z")     \
  _(STRADJUST_PRI,  "stradjust.pri")  \
  _(UNGEN_STKADJUST,"stackadjust")    \
  _(ENDPROC,        "endproc")        \
  _(FABS,           "fabs")           \
  _(FLOAT,          "float")          \
  _(FLOATADD,       "float.add")      \
  _(FLOATSUB,       "float.sub")      \
  _(FLOATMUL,       "float.mul")      \
  _(FLOATDIV,       "float.div")      \
  _(RND_TO_NEAREST, "round")          \
  _(RND_TO_FLOOR,   "floor")          \
  _(RND_TO_CEIL,    "ceil")           \
  _(RND_TO_ZERO,    "rndtozero")      \
  _(FLOATCMP,       "float.cmp")      \
  _(FLOAT_GT,       "float.gt")       \
  _(FLOAT_GE,       "float.ge")       \
  _(FLOAT_LT,       "float.lt")       \
  _(FLOAT_LE,       "float.le")       \
  _(FLOAT_NE,       "float.ne")       \
  _(FLOAT_EQ,       "float.eq")       \
  _(FLOAT_NOT,      "float.not")

enum OPCODE {
#define _(op, text) OP_##op,
  OPCODE_LIST(_)
#undef _
  OPCODES_TOTAL
};

namespace SourcePawn {
  void SpewOpcode(const sp_plugin_t *plugin, cell_t *start, cell_t *cip);
}

#endif //_INCLUDE_SOURCEPAWN_JIT_X86_OPCODES_H_
