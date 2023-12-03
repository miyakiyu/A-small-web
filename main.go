package main

import (
	"net/http"

	"github.com/gin-gonic/gin"

	"os/exec"
)

func main() {
	r := gin.Default()             //start the server
	r.LoadHTMLGlob("pages/*.html") //Load all file in the pages folder

	//home page
	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "main.html", gin.H{})
	})

	//PING AND SHOW THE RESULT FUNCCTION IS HERE!
	//get the ping request form
	r.GET("/ping", func(c *gin.Context) {
		c.HTML(http.StatusOK, "ping.html", nil)
	})

	//get the ping result
	r.POST("/pong", func(c *gin.Context) {
		ip := c.PostForm("ip")
		output, err := exec.Command("ping", "-c", "5", ip).CombinedOutput()
		if err != nil {
			println("Mission Failed successfully")
		}
		c.String(http.StatusOK, string(output))
	})

	//get the ping result and network flow dynamic
	//r.GET("/")
	//Gogo
	r.Run(":8080")
}
