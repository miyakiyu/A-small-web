package main

import (
	"net/http"

	"github.com/gin-gonic/gin"

	"os/exec"
)

func main() {
	r := gin.Default()             //start the server
	r.LoadHTMLGlob("pages/*.html") //Load all file in the pages folder
	r.Static("/css", "./css")      //Load the CSS file for website
	//Just for Test
	r.Use(func(c *gin.Context) {
		c.Header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
		c.Next()
	})

	//HOME PAGE
	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "main.html", gin.H{})
	})

	//PING
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

	//RESOLVER REQUEST
	//Web for Resolver
	r.GET("/resolver", func(c *gin.Context) {
		c.HTML(http.StatusOK, "resolver.html", nil)
	})
	//handle resolver request
	r.POST("/run-a-out", func(c *gin.Context) {
		nsip := c.PostForm("nsip")
		port := c.PostForm("port")
		dn := c.PostForm("dn")

		// Run the a.out command
		cmd := exec.Command("app/./ddos.o", nsip, port, dn)
		output, err := cmd.CombinedOutput()

		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}

		// Pass the output to the HTML template
		c.HTML(http.StatusOK, "resolver.html", gin.H{
			"Output": string(output),
		})
	})

	//

	//Gogo
	r.Run(":8080")
}
