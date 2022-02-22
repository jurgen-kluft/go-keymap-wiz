package qmk_keymap_wiz

import (
	glfw "github.com/jurgen-kluft/libglfw/package"
	imgui "github.com/jurgen-kluft/libimgui/package"
	xbase "github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xcode/denv"
	xjsmn "github.com/jurgen-kluft/xjsmn/package"
)

// GetPackage returns the package object of 'qmk-keymap-wiz'
func GetPackage() *denv.Package {
	// Dependencies
	xbasepkg := xbase.GetPackage()
	xjsmnpkg := xjsmn.GetPackage()
	imguipkg := imgui.GetPackage()
	glfwpkg := glfw.GetPackage()

	// The main (qmk-keymap-wiz) package
	mainpkg := denv.NewPackage("qmk-keymap-wiz")
	mainpkg.AddPackage(xbasepkg)
	mainpkg.AddPackage(xjsmnpkg)
	mainpkg.AddPackage(imguipkg)
	mainpkg.AddPackage(glfwpkg)

	// 'qmk-keymap-wiz' library
	mainapp := denv.SetupDefaultCppAppProject("qmk-keymap-wiz", "github.com\\jurgen-kluft\\qmk-keymap-wiz")

	mainapp.Dependencies = append(mainapp.Dependencies, xbasepkg.GetMainLib())
	mainapp.Dependencies = append(mainapp.Dependencies, xjsmnpkg.GetMainLib())
	mainapp.Dependencies = append(mainapp.Dependencies, imguipkg.GetMainLib())
	mainapp.Dependencies = append(mainapp.Dependencies, glfwpkg.GetMainLib())

	mainpkg.AddMainApp(mainapp)
	return mainpkg
}
