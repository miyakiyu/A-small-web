package main

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()        //start the server
	r.LoadHTMLGlob("pages/*") //Load all file in the pages folder

	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "main.html", gin.H{
			"title": "here is my home now",
		})
	})

	r.GET("/status", func(c *gin.Context) {
		c.HTML(http.StatusOK, "status.html", gin.H{})
	})

	r.GET("/ping", func(c *gin.Context) {
		c.HTML(http.StatusOK, "ping.html", gin.H{})
	})

	//Gogo
	r.Run(":8080")
}
