#pragma once
#include <stdint.h>


union disasm_flags_t {
	uint32_t data;
	struct {
		bool _66 : 1;
		bool _67 : 1;
		bool lock : 1;
		bool rep : 1;
		bool seg : 1;
		bool modrm : 1;
		bool sib : 1;
		bool op2 : 1;
		bool op3 : 1;
		bool vex : 1;
		bool rex : 1;
	};
};


union modrm_t {
	uint8_t data;
	struct {
		uint8_t rm : 3;
		uint8_t reg : 3;
		uint8_t mod : 2;
	};
};

union sib_t {
	uint8_t data;
	struct {
		uint8_t base : 3;
		uint8_t index : 3;
		uint8_t scale : 2;
	};
};

union rex_t {
	uint8_t data;
	struct {
		uint8_t b : 1;
		uint8_t x : 1;
		uint8_t r : 1;
		uint8_t w : 1;
		uint8_t pad : 4;
	};
};

#ifndef ASMLEN
#define ASMLEN

uint32_t asmlen(uint8_t* ptr, bool x64 = false)
{
    disasm_flags_t	disasm_flags{ 0 };
    modrm_t		disasm_modrm;
    sib_t		disasm_sib{ 0 };
    rex_t		disasm_rex{ 0 };
    uint32_t		disasm_opsize = 4;
    uint32_t		disasm_memsize = x64 ? 8 : 4;
    uint32_t		disasm_vexsize = 0;
    uint32_t		disasm_datasize = 0;
    uint32_t		disasm_len = 0;

next:
    uint8_t disasm_op1 = *ptr++;
    switch (disasm_op1)
    {
    case 0x00: case 0x01: case 0x02: case 0x03:
    case 0x10: case 0x11: case 0x12: case 0x13:
    case 0x20: case 0x21: case 0x22: case 0x23:
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x08: case 0x09: case 0x0A: case 0x0B:
    case 0x18: case 0x19: case 0x1A: case 0x1B:
    case 0x28: case 0x29: case 0x2A: case 0x2B:
    case 0x38: case 0x39: case 0x3A: case 0x3B:
    case 0x63:
    case 0x84: case 0x85: case 0x86: case 0x87:
    case 0x88: case 0x89: case 0x8A: case 0x8B:
    case 0x8C: case 0x8D: case 0x8E: case 0x8F:
    case 0xD0: case 0xD1: case 0xD2: case 0xD3:
    case 0xD8: case 0xD9: case 0xDA: case 0xDB:
    case 0xDC: case 0xDD: case 0xDE: case 0xDF:
    case 0xFE:
        disasm_flags.modrm = true;
        break;
    case 0x62:
        if (x64) return 0;
        disasm_flags.modrm = true;
        break;
    case 0xFF:
        if(x64){
            disasm_modrm.data = *ptr;
            switch (disasm_modrm.reg) {
            case 0x02: case 0x04:
                disasm_opsize = 8;
                break;
            case 0x06:
                disasm_opsize = disasm_flags._66 ? 2 : 8;
            }
        }
        disasm_flags.modrm = true;
        break;
    case 0xF6: case 0xF7:
        disasm_flags.modrm = true;
        if (*ptr & 0x38) break;
    case 0x04: case 0x05: case 0x0C: case 0x0D:
    case 0x14: case 0x15: case 0x1C: case 0x1D:
    case 0x24: case 0x25: case 0x2C: case 0x2D:
    case 0x34: case 0x35: case 0x3C: case 0x3D:
        if (disasm_op1 & 1)
            disasm_datasize += (disasm_opsize == 8 || disasm_opsize == 4) ? 4 : 2; //Iz
        else
            disasm_datasize += 1; //Ib
        break;
    case 0xD4: case 0xD5:
        if (x64) return 0;
    case 0x70: case 0x71: case 0x72: case 0x73:
    case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7A: case 0x7B:
    case 0x7C: case 0x7D: case 0x7E: case 0x7F:
    case 0x6A: case 0xA8:
    case 0xB0: case 0xB1: case 0xB2: case 0xB3:
    case 0xB4: case 0xB5: case 0xB6: case 0xB7:
    case 0xCD:
    case 0xE4: case 0xE5: case 0xE6: case 0xE7:
    case 0xEB:
    case 0xE0: case 0xE1: case 0xE2: case 0xE3:
        disasm_datasize += 1; // Ib / Jb
        break;
    case 0x26: case 0x2E: case 0x36: case 0x3E:
    case 0x64: case 0x65: // segment register select prefix
        if (disasm_flags.seg) return 0;
        disasm_flags.seg = true;
        goto next;
    case 0xF0:
        if (disasm_flags.lock) return 0;  // lock prefix
        disasm_flags.lock = true;
        goto next;
    case 0xF2: case 0xF3:
        if (disasm_flags.rep) return 0; // repeat prefix
        disasm_flags.rep = true;
        goto next;
    case 0x66:
        if (disasm_flags._66) return 0; // operand size override prefix
        disasm_flags._66 = true;
        disasm_opsize = 2;
        goto next;
    case 0x67:
        if (disasm_flags._67) return 0; // address size override prefix
        disasm_flags._67 = true;
        disasm_memsize = 4;
        goto next;
    case 0xC4: case 0xC5: {
        disasm_modrm.data = *ptr;
        if (disasm_modrm.mod == 0x03 || x64) { // it's VEX prefix
            if (disasm_flags.vex) return 0;
            disasm_flags.vex = true;
            disasm_vexsize = (disasm_op1 == 0xC4) ? 2 : 1;
            ptr += disasm_vexsize;
            goto next;
        }
        else { // it's opcode with constant operator
            disasm_flags.modrm = true;
            break;
        }
    }
    case 0x40: case 0x41: case 0x42: case 0x43:
    case 0x44: case 0x45: case 0x46: case 0x47:
    case 0x48: case 0x49: case 0x4A: case 0x4B:
    case 0x4C: case 0x4D: case 0x4E: case 0x4F:
        if (x64) {
            // rex prefix
            if (disasm_flags.rex) return 0;
            disasm_flags.rex = true;
            disasm_rex.data = disasm_op1;
            if (disasm_rex.w)
                disasm_opsize = 8;
            goto next;
        }
        break;
    case 0x82:
        if (x64) return 0;
    case 0x80: case 0x83: case 0xC0:
    case 0xC1: case 0xC6: case 0x6B:
        disasm_datasize += 1; // modrm and Ib
        disasm_flags.modrm = true;
        break;
    case 0x69: case 0x81: case 0xC7:
        disasm_datasize += (disasm_opsize == 8 || disasm_opsize == 4) ? 4 : 2; // modrm and Iz
        disasm_flags.modrm = true;
        break;
    case 0xA0: case 0xA1: case 0xA2: case 0xA3:
        disasm_memsize += disasm_memsize; // O, size is 16/32/64 base on address size
        break;
    case 0x68: case 0xA9: case 0xE8: case 0xE9:
        disasm_datasize += (disasm_opsize == 8 || disasm_opsize == 4) ? 4 : 2; // Iz
        break;
    case 0xB8: case 0xB9: case 0xBA: case 0xBB:
    case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        disasm_datasize += disasm_opsize; //Iv
        break;
    case 0xC2: case 0xCA:
        disasm_datasize += 2; // Iw
        break;
    case 0xC8:
        disasm_datasize += 3; // Iw with Ib
        break;
    case 0x9A: case 0xEA:
        if (x64) return 0;
        disasm_datasize += 2 + disasm_opsize; // Ap, size is 32/48/80 base on operand size
        break;
    case 0x06: case 0x07: case 0x0E:
    case 0x16: case 0x17: case 0x1E: case 0x1F:
    case 0x27: case 0x2F: case 0x37: case 0x3F:
    case 0x60: case 0x61: 
    case 0xCE:
    case 0xD6:
        if(x64) return 0; // bad
        break;  // it's opcode with constant operator
    case 0x0F: { // 2-byte opcode escape
        uint8_t disasm_op2 = *ptr++;
        disasm_flags.op2 = true;
        switch (disasm_op2)
        {
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x0D:
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x1A: case 0x1B: case 0x1F:
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x28: case 0x29: case 0x2A: case 0x2B:
        case 0x2C: case 0x2D: case 0x2E: case 0x2F:
        case 0x40: case 0x41: case 0x42: case 0x43:
        case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B:
        case 0x4C: case 0x4D: case 0x4E: case 0x4F:
        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        case 0x60: case 0x61: case 0x62: case 0x63:
        case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x68: case 0x69: case 0x6A: case 0x6B:
        case 0x6C: case 0x6D: case 0x6E: case 0x6F:
        case 0x74: case 0x75: case 0x76: case 0x78:
        case 0x79: case 0x7C: case 0x7D: case 0x7E:
        case 0x7F:
        case 0x90: case 0x91: case 0x92: case 0x93:
        case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9A: case 0x9B:
        case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        case 0xA3: case 0xA5: case 0xAB: case 0xAD:
        case 0xAE: case 0xAF:
        case 0xB0: case 0xB1: case 0xB2: case 0xB3:
        case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        case 0xB8: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        case 0xC0: case 0xC1: case 0xC3: case 0xC7:
        case 0xD0: case 0xD1: case 0xD2: case 0xD3:
        case 0xD4: case 0xD5: case 0xD6: case 0xD7:
        case 0xD8: case 0xD9: case 0xDA: case 0xDB:
        case 0xDC: case 0xDD: case 0xDE: case 0xDF:
        case 0xE0: case 0xE1: case 0xE2: case 0xE3:
        case 0xE4: case 0xE5: case 0xE6: case 0xE7:
        case 0xE8: case 0xE9: case 0xEA: case 0xEB:
        case 0xEC: case 0xED: case 0xEE: case 0xEF:
        case 0xF0: case 0xF1: case 0xF2: case 0xF3:
        case 0xF4: case 0xF5: case 0xF6: case 0xF7:
        case 0xF8: case 0xF9: case 0xFA: case 0xFB:
        case 0xFC: case 0xFD: case 0xFE:
            disasm_flags.modrm = true;
            break;
        case 0x05: case 0x06: case 0x07: case 0x08:
        case 0x09:
        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x37:
        case 0x77:
        case 0xA0: case 0xA1: case 0xA2: case 0xA8:
        case 0xA9: case 0xAA:
        case 0xC8: case 0xC9: case 0xCA: case 0xCB:
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
            break; //const operation
        case 0x80: case 0x81: case 0x82: case 0x83:
        case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8A: case 0x8B:
        case 0x8C: case 0x8D: case 0x8E: case 0x8F:
            disasm_datasize += (disasm_opsize == 8 || disasm_opsize == 4) ? 4 : 2;
            break;
        case 0x70: case 0x71: case 0x72: case 0x73:
        case 0xA4: case 0xAC:
        case 0xBA:
        case 0xC2: case 0xC4: case 0xC5: case 0xC6:
            disasm_datasize += 1;
            disasm_flags.modrm = true;
            break;
        case 0x38: { // 3-byte opcode escape
            uint8_t disasm_op3 = *ptr++;
            disasm_flags.op3 = true;
            switch (disasm_op3)
            {
            case 0x00: case 0x01: case 0x02: case 0x03:
            case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0A: case 0x0B:
            case 0x0C: case 0x0D: case 0x0E: case 0x0F:
            case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1A:
            case 0x1C: case 0x1D: case 0x1E:
            case 0x20: case 0x21: case 0x22: case 0x23:
            case 0x24: case 0x25: case 0x28: case 0x29:
            case 0x2A: case 0x2B: case 0x2C: case 0x2D:
            case 0x2E: case 0x2F:
            case 0x30: case 0x31: case 0x32: case 0x33:
            case 0x34: case 0x35: case 0x36: case 0x37:
            case 0x38: case 0x39: case 0x3A: case 0x3B:
            case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            case 0x40: case 0x41:
            case 0x45: case 0x46: case 0x47:
            case 0x58: case 0x59: case 0x5A:
            case 0x78: case 0x79:
            case 0x80: case 0x81: case 0x82:
            case 0x8C: case 0x8E:
            case 0x90: case 0x91: case 0x92: case 0x93:
            case 0x96: case 0x97: case 0x98: case 0x99:
            case 0x9A: case 0x9B: case 0x9C: case 0x9D:
            case 0x9E: case 0x9F:
            case 0xA6: case 0xA7: case 0xA8: case 0xA9:
            case 0xAA: case 0xAB: case 0xAC: case 0xAD:
            case 0xAE: case 0xAF:
            case 0xB6: case 0xB7: case 0xB8: case 0xB9:
            case 0xBA: case 0xBB: case 0xBC: case 0xBD:
            case 0xBE: case 0xBF:
            case 0xC8: case 0xC9: case 0xCA: case 0xCB:
            case 0xCC: case 0xCD:
            case 0xDB: case 0xDC: case 0xDD: case 0xDE:
            case 0xDF:
            case 0xF0: case 0xF1: case 0xF2: case 0xF3:
            case 0xF5: case 0xF6: case 0xF7:
                disasm_flags.modrm = true;
                break;
            case 0x10: case 0x13:
                disasm_datasize += 1; // modrm and Ib
                disasm_flags.modrm = true;
                break;
            default: // bad
                return 0;
            }
        }
        case 0x3A: { // 3-byte opcode escape
            uint8_t disasm_op3 = *ptr++;
            disasm_flags.op3 = true;
            switch (disasm_op3)
            {
            case 0x00: case 0x01: case 0x02: case 0x04:
            case 0x05: case 0x06: case 0x08: case 0x09:
            case 0x0A: case 0x0B: case 0x0C: case 0x0D:
            case 0x0E: case 0x0F:
            case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1D: case 0x20:
            case 0x21: case 0x22: case 0x38: case 0x39:
            case 0x40: case 0x41: case 0x42: case 0x44:
            case 0x46: case 0x4A: case 0x4B: case 0x4C:
            case 0x60: case 0x61: case 0x62: case 0x63:
            case 0xCC: case 0xDF: case 0xF0:
                disasm_datasize += 1; // modrm and Ib
                disasm_flags.modrm = true;
                break;
            default: // bad
                return 0;
            }
        }
        default:
            return 0;
        }
    }
    }

    if (disasm_flags.modrm) { // has ModRM
        disasm_modrm.data = *ptr++;
        // mod: 11 = register other = memory, if it's register, ignore
        if (disasm_modrm.mod != 0x3) {
            if (disasm_modrm.mod == 0x01) {
                disasm_datasize += 1; // [any + disp8]
            }
            if (disasm_memsize == 0x02) { // in 16 bits mode
                if (disasm_modrm.rm == 0x06 || disasm_modrm.mod == 0x02) {
                    disasm_datasize += 2; // [any + disp16] or [disp16]
                }
            }
            else {
                if (disasm_modrm.rm == 0x05 || disasm_modrm.mod == 0x02) {
                    disasm_datasize += 4; // [any + disp32] or [disp32]
                }
                if (disasm_modrm.rm == 0x04) { // has SIB
                    disasm_flags.sib = true;
                    disasm_sib.data = *ptr;
                    if (disasm_sib.base == 0x05) {
                        if (disasm_modrm.mod == 0x01) {
                            disasm_datasize += 2;
                        }
                        else {
                            disasm_datasize += 4;
                        }
                    }
                }
            }
        }
    }
    disasm_len = disasm_flags._66 + disasm_flags._67 + disasm_flags.lock + disasm_flags.rep + disasm_flags.seg + disasm_flags.vex + disasm_vexsize + 1 + disasm_flags.op2 + disasm_flags.op3 + disasm_flags.modrm + disasm_flags.sib + disasm_datasize;
    return disasm_len;
}

#endif
