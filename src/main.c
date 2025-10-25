#include "pmd.h"
#include "msg.h"

#include <sanitizer/lsan_interface.h>

int main(int argc, char** argv)
{
	msg_set_level(MSG_DEBUG);
	__lsan_disable();

	pmd p = {0};
	pmd_init(&p);

	var source_tree = create_tree("test.tau");
	bool exit = pmd_eval_bool(&p, source_tree);

	if (!exit)
	{
		warn("testfile failed");
	}

	pmd_free(&p);
	return !exit;
}
