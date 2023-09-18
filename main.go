package main

import (
	"net/http"
	"os/exec"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()        //start the server
	r.LoadHTMLGlob("pages/*") //Load all file in the pages folder

	r.GET("/hmm", func(c *gin.Context) {
		c.HTML(http.StatusOK, "main.html", gin.H{
			"title": "here is my home now",
		})
	})

	r.GET("/", func(c *gin.Context) {
		//execute program
		cmd := exec.Command("./add")
		//get the output and error
		out, err := cmd.Output()
		//error handler
		if err != nil {
			c.String(http.StatusInternalServerError, "Error executing C program")
			return
		}
		//show result
		result := string(out)
		c.String(http.StatusOK, "Result: %s", result)
	})

	r.GET("/status", func(c *gin.Context) {
		c.HTML(http.StatusOK, "status.html", gin.H{})
	})

	//Gogo
	r.Run(":8080")
}
