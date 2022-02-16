package main

import (
	"strconv"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/layout"
	"fyne.io/fyne/v2/widget"
)

type calc struct {
	equation string

	output  *widget.Label
	buttons map[string]*widget.Button
	window  fyne.Window
}

func (c *calc) display(newtext string) {
	c.equation = newtext
	c.output.SetText(newtext)
}

func (c *calc) character(char rune) {
	c.display(c.equation + string(char))
}

func (c *calc) digit(d int) {
	c.character(rune(d) + '0')
}

func (c *calc) clear() {
	c.display("")
}

func (c *calc) evaluate() {
	c.display("error")
}

func (c *calc) addButton(text string, action func()) *widget.Button {
	button := widget.NewButton(text, action)
	c.buttons[text] = button
	return button
}

func (c *calc) digitButton(number int) *widget.Button {
	str := strconv.Itoa(number)
	return c.addButton(str, func() {
		c.digit(number)
	})
}

func (c *calc) charButton(char rune) *widget.Button {
	return c.addButton(string(char), func() {
		c.character(char)
	})
}

func (c *calc) onTypedRune(r rune) {
	if r == 'c' {
		r = 'C' // The button is using a capital C.
	}

	if button, ok := c.buttons[string(r)]; ok {
		button.OnTapped()
	}
}

func (c *calc) onTypedKey(ev *fyne.KeyEvent) {
	if ev.Name == fyne.KeyReturn || ev.Name == fyne.KeyEnter {
		c.evaluate()
	} else if ev.Name == fyne.KeyBackspace && len(c.equation) > 0 {
		c.display(c.equation[:len(c.equation)-1])
	}
}

func (c *calc) onPasteShortcut(shortcut fyne.Shortcut) {
	content := shortcut.(*fyne.ShortcutPaste).Clipboard.Content()
	if _, err := strconv.ParseFloat(content, 64); err != nil {
		return
	}

	c.display(c.equation + content)
}

func (c *calc) onCopyShortcut(shortcut fyne.Shortcut) {
	shortcut.(*fyne.ShortcutCopy).Clipboard.SetContent(c.equation)
}

func (c *calc) loadUI(app fyne.App) {
	c.output = &widget.Label{Alignment: fyne.TextAlignTrailing}
	c.output.TextStyle.Monospace = true

	equals := c.addButton("=", c.evaluate)
	equals.Importance = widget.HighImportance

	c.window = app.NewWindow("Keyboard Configurator")

	// create 64 keys and add them to the keys array
	layer := container.New(layout.NewGridLayout(16))
	for i := 0; i < 64; i++ {
		key := c.digitButton(i)
		layer.Add(key)
	}

	// need a special layout that can position buttons according to the layout of the keyboard.
	//

	c.window.SetContent(layer)

	c.window.Canvas().SetOnTypedRune(c.onTypedRune)
	c.window.Canvas().SetOnTypedKey(c.onTypedKey)
	c.window.Canvas().AddShortcut(&fyne.ShortcutCopy{}, c.onCopyShortcut)
	c.window.Canvas().AddShortcut(&fyne.ShortcutPaste{}, c.onPasteShortcut)
	c.window.Resize(fyne.NewSize(200, 300))
	c.window.Show()
}

func newWiz() *calc {
	return &calc{
		buttons: make(map[string]*widget.Button, 19),
	}
}
