#include "pmd.h"
#include "msg.h"
#include "flag.h"

#include <sanitizer/lsan_interface.h>

int main(int argc, char** argv)
{
    const char *file_path = create_flag(argc, argv, char *, NULL, "This is the file");
    if (argc < 2) {
        help_flag();
        return 0;
    }

	msg_set_level(MSG_DEBUG);
	__lsan_disable();

	pmd p = {0};
	pmd_init(&p);

	var source_tree = create_tree(file_path);

    pmd_free(&p);
	return !exit;
}
