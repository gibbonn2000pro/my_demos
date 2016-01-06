package main
import (
	// "fmt"
	"strconv"
	"sync"
	"runtime"
)
import (
	"executor"
)

func main() {
	runtime.GOMAXPROCS(1)
	var wg sync.WaitGroup
	for i := 0; i < 3000; i++ {
		script := genScript(i)
		go executor.GoRunScript([]byte(script))
		wg.Add(1)
	}
	wg.Wait()
}


func genScript(id int) string {
	return `
var proc = function(code) {
	print(code + ": before_sleep");
	sleep(3);
	print(code+": after_sleep");
};

print("proc ` + strconv.Itoa(id) + ` start");
while(true) {
	proc(` + strconv.Itoa(id) + `);
}`
}