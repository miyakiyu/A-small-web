package main

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.LoadHTMLGlob("pages/*") //Load all file in the pages folder
	r.GET("/main", func(c *gin.Context) {
		c.HTML(http.StatusOK, "main.html", gin.H{
			"title": "here is my home now",
		})
	})
	r.Run(":8080")
}
