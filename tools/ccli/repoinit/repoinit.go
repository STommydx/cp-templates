package repoinit

import (
	"embed"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strings"
	"text/template"

	"github.com/STommydx/cp-templates/tools/ccli/output"
	"github.com/go-git/go-git/v5"
)

type Settings struct {
	Directory             string
	NumberOfMainPrograms  int
	TemplateRepositoryUrl string
	IncludeTestlib        bool
}

//go:embed templates/*
var templatesFS embed.FS

// RunWithOutput initializes a new repository with the given outputter for messaging
func RunWithOutput(settings Settings, out output.Outputter) error {
	spinner := out.WithSpinner("Create repository directory")
	if err := spinner.Start(); err != nil {
		return err
	}

	if _, err := os.Stat(settings.Directory); os.IsNotExist(err) {
		err := os.Mkdir(settings.Directory, os.ModePerm)
		if err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to create directory: %w", err)
		}
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	spinner = out.WithSpinner("Create main program files")
	if err := spinner.Start(); err != nil {
		return err
	}

	copyFile := func(source, destination string) error {
		templateFile, err := templatesFS.Open(path.Join("templates", source))
		if err != nil {
			return fmt.Errorf("failed to open template file: %w", err)
		}
		defer templateFile.Close()

		file, err := os.Create(filepath.Join(settings.Directory, destination))
		if err != nil {
			return fmt.Errorf("failed to create file: %w", err)
		}
		defer file.Close()

		if _, err := io.Copy(file, templateFile); err != nil {
			return fmt.Errorf("failed to copy template file: %w", err)
		}
		return nil
	}

	if !settings.IncludeTestlib {
		for i := 1; i <= settings.NumberOfMainPrograms; i++ {
			if err := copyFile("main.cpp", fmt.Sprintf("%c.cpp", rune(64+i))); err != nil {
				spinner.StopFail()
				return err
			}
		}
	} else {
		if err := copyFile("main.cpp", "solution.cpp"); err != nil {
			spinner.StopFail()
			return err
		}
		if err := copyFile("gen.cpp", "gen.cpp"); err != nil {
			spinner.StopFail()
			return err
		}
		if err := copyFile("checker.cpp", "checker.cpp"); err != nil {
			spinner.StopFail()
			return err
		}
		if err := copyFile("val.cpp", "val.cpp"); err != nil {
			spinner.StopFail()
			return err
		}
		if err := copyFile("interactor.cpp", "interactor.cpp"); err != nil {
			spinner.StopFail()
			return err
		}
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	templateFiles := []string{
		".gitignore",
		".clang-format",
	}

	spinner = out.WithSpinner("Create template files")
	if err := spinner.Start(); err != nil {
		return err
	}

	for _, templateFilename := range templateFiles {
		templateFile, err := templatesFS.Open(path.Join("templates", templateFilename))
		if err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to open template file %s: %w", templateFilename, err)
		}
		defer templateFile.Close()

		file, err := os.Create(path.Join(settings.Directory, templateFilename))
		if err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to create file: %w", err)
		}
		defer file.Close()

		if _, err := io.Copy(file, templateFile); err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to copy template file: %w", err)
		}
	}

	cmakeConfig := struct {
		ProjectName string
		SourceFiles []struct {
			Input  string
			Output string
		}
	}{
		ProjectName: path.Base(settings.Directory),
	}

	files, err := os.ReadDir(settings.Directory)
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to read directory: %w", err)
	}

	for _, file := range files {
		if filepath.Ext(file.Name()) == ".cpp" {
			cmakeConfig.SourceFiles = append(cmakeConfig.SourceFiles, struct {
				Input  string
				Output string
			}{
				Input:  file.Name(),
				Output: strings.ReplaceAll(file.Name(), ".cpp", ".out"),
			})
		}
	}

	cmakeListFile, err := os.Create(path.Join(settings.Directory, "CMakeLists.txt"))
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to create CMakeLists.txt file: %w", err)
	}
	defer cmakeListFile.Close()

	cmakeListTemplate := template.Must(template.ParseFS(templatesFS, "templates/CMakeLists.txt.tmpl"))
	if err := cmakeListTemplate.Execute(cmakeListFile, cmakeConfig); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to execute CMakeLists.txt template: %w", err)
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	spinner = out.WithSpinner("Initialize git repository")
	if err := spinner.Start(); err != nil {
		return err
	}

	repo, err := git.PlainInit(settings.Directory, false)
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to initialize git repository: %w", err)
	}

	worktree, err := repo.Worktree()
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to get worktree of git repository: %w", err)
	}

	if _, err := worktree.Add("."); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to add files to git repository: %w", err)
	}

	if _, err := worktree.Commit("feat: initial repository with main templates", &git.CommitOptions{}); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to commit main files to git repository: %w", err)
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	spinner = out.WithSpinner("Download templates")
	if err := spinner.Start(); err != nil {
		return err
	}

	addSubModuleCmd := exec.Command("git", "submodule", "add", settings.TemplateRepositoryUrl, "templates")
	addSubModuleCmd.Dir = settings.Directory
	if err := addSubModuleCmd.Run(); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to add submodule to git repository: %w", err)
	}

	repo, err = git.PlainOpen(settings.Directory)
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to open git repository: %w", err)
	}

	worktree, err = repo.Worktree()
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to get worktree of git repository: %w", err)
	}

	if _, err := worktree.Add("."); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to add files to git repository: %w", err)
	}

	if _, err := worktree.Commit("feat: add templates submodule", &git.CommitOptions{}); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to commit template files to git repository: %w", err)
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	if settings.IncludeTestlib {
		spinner = out.WithSpinner("Download testlib")
		if err := spinner.Start(); err != nil {
			return err
		}

		addSubModuleCmd := exec.Command("git", "submodule", "add", "git@github.com:MikeMirzayanov/testlib.git", "testlib")
		addSubModuleCmd.Dir = settings.Directory
		if err := addSubModuleCmd.Run(); err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to add submodule to git repository: %w", err)
		}

		if err := os.Symlink(filepath.Join("testlib", "testlib.h"), filepath.Join(settings.Directory, "testlib.h")); err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to create symlink to testlib.h: %w", err)
		}

		repo, err := git.PlainOpen(settings.Directory)
		if err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to open git repository: %w", err)
		}

		worktree, err := repo.Worktree()
		if err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to get worktree of git repository: %w", err)
		}

		if _, err := worktree.Add("."); err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to add files to git repository: %w", err)
		}

		if _, err := worktree.Commit("feat: add testlib submodule", &git.CommitOptions{}); err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to commit template files to git repository: %w", err)
		}

		if err := spinner.Stop(); err != nil {
			return err
		}
	}

	return nil
}

// Run initializes a new repository using the default terminal outputter (backward compatibility)
func Run(settings Settings) error {
	return RunWithOutput(settings, output.DefaultTerminalOutputter())
}
