package main

import "fyne.io/fyne/v2/app"

func main() {
	app := app.New()

	c := newWiz()
	c.loadUI(app)
	app.Run()
}
