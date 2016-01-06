#include "duktape.h"

extern void GoSleepFunc(int s);

int sleep(duk_context *ctx) {
	int s = duk_require_int(ctx, 0);
	GoSleepFunc(s);
	return 0;
}


void RunScript(char* script, size_t script_len) {
	duk_context *ctx = duk_create_heap_default();

	duk_push_global_object(ctx);
	duk_push_c_function(ctx, sleep, 1);
	duk_put_prop_string(ctx, -2, "sleep");
	duk_pop(ctx);

	duk_eval_lstring_noresult(ctx, script, script_len);
	duk_destroy_heap(ctx);
}
