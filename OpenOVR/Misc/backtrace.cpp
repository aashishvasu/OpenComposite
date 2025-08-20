#include "backtrace.h"
#include "logging.h"

#if defined(OC_BACKTRACE) && defined(__unix__)
#include <csignal>
#include <cstdlib>
#include <libunwind.h>

inline static void dump_registers(unw_cursor_t* cursor)
{
#if defined(__aarch64__)
	constexpr int regs[] = {
		UNW_AARCH64_X0,
		UNW_AARCH64_X1,
		UNW_AARCH64_X2,
		UNW_AARCH64_X3,
		UNW_AARCH64_X4,
		UNW_AARCH64_X5,
		UNW_AARCH64_X6,
		UNW_AARCH64_X7,
		UNW_AARCH64_X8,
		UNW_AARCH64_X9,
		UNW_AARCH64_X10,
		UNW_AARCH64_X11,
		UNW_AARCH64_X12,
		UNW_AARCH64_X13,
		UNW_AARCH64_X14,
		UNW_AARCH64_X15,
		UNW_AARCH64_X16,
		UNW_AARCH64_X17,
		UNW_AARCH64_X18,
		UNW_AARCH64_X19,
		UNW_AARCH64_X20,
		UNW_AARCH64_X21,
		UNW_AARCH64_X22,
		UNW_AARCH64_X23,
		UNW_AARCH64_X24,
		UNW_AARCH64_X25,
		UNW_AARCH64_X26,
		UNW_AARCH64_X27,
		UNW_AARCH64_X28,
		UNW_AARCH64_X29,
		UNW_AARCH64_X30,
	};
	constexpr const char* names[] = {
		"x0",
		"x1",
		"x2",
		"x3",
		"x4",
		"x5",
		"x6",
		"x7",
		"x8",
		"x9",
		"x10",
		"x11",
		"x12",
		"x13",
		"x14",
		"x15",
		"x16",
		"x17",
		"x18",
		"x19",
		"x20",
		"x21",
		"x22",
		"x23",
		"x24",
		"x25",
		"x26",
		"x27",
		"x28",
		"x29",
		"x30",
	};
#elif defined(__x86_64__)
	constexpr int regs[] = {
		UNW_X86_64_RAX,
		UNW_X86_64_RBX,
		UNW_X86_64_RCX,
		UNW_X86_64_RDX,
		UNW_X86_64_RSI,
		UNW_X86_64_RDI,
		UNW_X86_64_RBP,
		UNW_X86_64_RSP,
		UNW_X86_64_R8,
		UNW_X86_64_R9,
		UNW_X86_64_R10,
		UNW_X86_64_R11,
		UNW_X86_64_R12,
		UNW_X86_64_R13,
		UNW_X86_64_R14,
		UNW_X86_64_R15,
	};
	constexpr const char* names[] = {
		"RAX",
		"RBX",
		"RCX",
		"RDX",
		"RSI",
		"RDI",
		"RBP",
		"RSP",
		"R8",
		"R9",
		"R10",
		"R11",
		"R12",
		"R13",
		"R14",
		"R15",
	};
#elif defined(__i386__)
	constexpr int regs[] = {
		UNW_X86_EAX,
		UNW_X86_EBX,
		UNW_X86_ECX,
		UNW_X86_EDX,
		UNW_X86_ESI,
		UNW_X86_EDI,
		UNW_X86_EBP,
		UNW_X86_ESP,
	};
	constexpr const char* names[] = {
		"EAX",
		"EBX",
		"ECX",
		"EDX",
		"ESI",
		"EDI",
		"EBP",
		"ESP",
	};
#else
#error "Unsupported architecture"
#endif

	for (size_t i = 0; i < sizeof(regs) / sizeof(*regs); ++i) {
		unw_word_t value;
		if (unw_get_reg(cursor, regs[i], &value) < 0) {
			oovr_printf_safe("\t%s\t\t???\n", names[i]);
		} else {
			oovr_printf_safe("\t%s\t\t0x%lx\n", names[i], (long)value);
		}
	}
}

static void oovr_dump_backtrace(unw_cursor_t* cursor)
{
	int r;
	do {
		char func_name[256];
		unw_word_t offset;
		r = unw_get_proc_name(cursor, func_name, sizeof(func_name), &offset);
		if (r < 0) {
			if (r == -UNW_ENOINFO) {
				unw_proc_info_t info;
				unw_word_t ip;
				if (unw_get_proc_info(cursor, &info) < 0 || unw_get_reg(cursor, UNW_REG_IP, &ip) < 0) {
					oovr_printf_safe("\t???\n");
				} else {
					oovr_printf_safe("\t0x%lx + 0x%lx\n", info.start_ip, ip - info.start_ip);
				}
			} else {
				oovr_printf_safe("\t*failed to read frame info*\n");
			}
		} else {
			oovr_printf_safe("\t%s + 0x%lx\n", func_name, offset);
		}
	} while ((r = unw_step(cursor)) > 0);

	switch (r) {
	case -UNW_ENOINFO:
		oovr_printf_safe("\tNo frame info\n");
		break;
	case -UNW_EBADFRAME:
		oovr_printf_safe("\tBad frame\n");
		break;
	case -UNW_EINVALIDIP:
		oovr_printf_safe("\tInvalid IP\n");
		break;
	case -UNW_EBADVERSION:
		oovr_printf_safe("\tBad version\n");
		break;
	case -UNW_EUNSPEC:
		oovr_printf_safe("\tUnspecified error\n");
		break;
	case -UNW_ESTOPUNWIND:
	default:
		break;
	}

	oovr_flush_safe();
}

void oovr_dump_backtrace()
{
	int r;
	unw_cursor_t cursor;
	unw_context_t context;

	r = unw_getcontext(&context);
	if (r < 0) {
		goto fail;
	}

	r = unw_init_local(&cursor, &context);
	if (r < 0) {
		goto fail;
	}

	oovr_printf_safe("===BACKTRACE===\n");
	oovr_dump_backtrace(&cursor);
	return;

fail:
	oovr_printf_safe("===BACKTRACE===\n");
	oovr_printf_safe("\tFailed to unwind context\n");
	oovr_flush_safe();
}

inline static void oovr_dump_crash_info(ucontext_t* context)
{
	int r;
	unw_cursor_t cursor;
	r = unw_init_local2(&cursor, context, UNW_INIT_SIGNAL_FRAME);
	if (r < 0) {
		oovr_printf_safe("\tFailed to unwind context\n");
		return;
	}

	oovr_printf_safe("===REGISTERS===\n");
	dump_registers(&cursor);

	oovr_printf_safe("===BACKTRACE===\n");
	oovr_dump_backtrace(&cursor);

	oovr_flush_safe();
}

static void sigsegv_handler(int, siginfo_t*, void* context)
{
	oovr_printf_safe("===SIGSEGV===\n");
	oovr_dump_crash_info((ucontext_t*)context);
	// Uninstall our crash handler and let it create a core dump
	signal(SIGSEGV, SIG_DFL);
}

void oovr_install_crash_handler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_flags = SA_SIGINFO | SA_ONSTACK;
	action.sa_sigaction = sigsegv_handler;
	sigaction(SIGSEGV, &action, NULL);
}
#else
void oovr_dump_backtrace()
{
	oovr_printf_safe("===BACKTRACE===\n");
	oovr_printf_safe("\tNot available\n");
	oovr_flush_safe();
}

void oovr_install_crash_handler() {}
#endif
