package main

import (
	"fmt"
	"os"
	"os/exec"
)

func main() {
	host := os.Args[1]
	output, err := exec.Command("ping", "-c", "5", host).CombinedOutput()

	if err != nil {
		print("Mission failed successfully!")
	}
	fmt.Println(string(output))
}
