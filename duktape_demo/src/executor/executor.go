package executor

/*
#include <stdlib.h>
void RunScript(char* script, size_t script_len);
*/
import "C"

import (
	"unsafe"
	"time"
)

func GoRunScript(script []byte) {
	C.RunScript((*C.char)(unsafe.Pointer(&script[0])), C.size_t(len(script)))
}


//export GoSleepFunc
func GoSleepFunc(s int) {
	time.Sleep(time.Duration(s) * time.Second)
}