package qmk_keymap_wiz

import (
	glfw "github.com/jurgen-kluft/libglfw/package"
	imgui "github.com/jurgen-kluft/libimgui/package"
	xbase "github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xcode/denv"
	xjson "github.com/jurgen-kluft/xjson/package"
)

// GetPackage returns the package object of 'qmk-keymap-wiz'
func GetPackage() *denv.Package {
	// Dependencies
	xbasepkg := xbase.GetPackage()
	xjsonpkg := xjson.GetPackage()
	imguipkg := imgui.GetPackage()
	glfwpkg := glfw.GetPackage()

	// The main (qmk-keymap-wiz) package
	mainpkg := denv.NewPackage("qmk-keymap-wiz")
	mainpkg.AddPackage(xbasepkg)
	mainpkg.AddPackage(xjsonpkg)
	mainpkg.AddPackage(imguipkg)
	mainpkg.AddPackage(glfwpkg)

	// 'qmk-keymap-wiz' library
	mainapp := denv.SetupDefaultCppAppProject("qmk-keymap-wiz", "github.com\\jurgen-kluft\\qmk-keymap-wiz")

	mainapp.Dependencies = append(mainapp.Dependencies, xbasepkg.GetMainLib())
	mainapp.Dependencies = append(mainapp.Dependencies, xjsonpkg.GetMainLib())
	mainapp.Dependencies = append(mainapp.Dependencies, imguipkg.GetMainLib())
	mainapp.Dependencies = append(mainapp.Dependencies, glfwpkg.GetMainLib())

	if denv.OS == denv.OS_WINDOWS {
		mainapp.AddLibrary("opengl32.lib;gdi32.lib;shell32.lib")
	}

	mainpkg.AddMainApp(mainapp)
	return mainpkg
}
