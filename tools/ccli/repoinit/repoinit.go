package repoinit

import (
	"embed"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path"
	"path/filepath"

	"github.com/STommydx/cp-templates/tools/ccli/spinner"
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

func Run(settings Settings) error {
	if err := spinner.Run("Create repository directory", func() error {
		if _, err := os.Stat(settings.Directory); os.IsNotExist(err) {
			err := os.Mkdir(settings.Directory, os.ModePerm)
			if err != nil {
				return fmt.Errorf("failed to create directory: %w", err)
			}
		}
		return nil
	}); err != nil {
		return err
	}
	if err := spinner.Run("Create main program files", func() error {
		copyFile := func(source, destination string) error {
			templateFile, err := templatesFS.Open(filepath.Join("templates", source))
			if err != nil {
				return fmt.Errorf("failed to open template file: %w", err)
			}
			defer templateFile.Close()
			file, err := os.Create(path.Join(settings.Directory, destination))
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
					return err
				}
			}
		} else {
			if err := copyFile("main.cpp", "solution.cpp"); err != nil {
				return err
			}
			if err := copyFile("gen.cpp", "gen.cpp"); err != nil {
				return err
			}
			if err := copyFile("checker.cpp", "checker.cpp"); err != nil {
				return err
			}
			if err := copyFile("val.cpp", "val.cpp"); err != nil {
				return err
			}
			if err := copyFile("interactor.cpp", "interactor.cpp"); err != nil {
				return err
			}
		}
		return nil
	}); err != nil {
		return err
	}
	templateFiles := []string{
		".gitignore",
		".clang-format",
	}
	if err := spinner.Run("Create template files", func() error {
		for _, templateFilename := range templateFiles {
			templateFile, err := templatesFS.Open(path.Join("templates", templateFilename))
			if err != nil {
				return fmt.Errorf("failed to open template file %s: %w", templateFilename, err)
			}
			defer templateFile.Close()
			file, err := os.Create(path.Join(settings.Directory, templateFilename))
			if err != nil {
				return fmt.Errorf("failed to create file: %w", err)
			}
			defer file.Close()
			if _, err := io.Copy(file, templateFile); err != nil {
				return fmt.Errorf("failed to copy template file: %w", err)
			}
		}
		return nil
	}); err != nil {
		return err
	}
	if err := spinner.Run("Initialize git repository", func() error {
		repo, err := git.PlainInit(settings.Directory, false)
		if err != nil {
			return fmt.Errorf("failed to initialize git repository: %w", err)
		}
		worktree, err := repo.Worktree()
		if err != nil {
			return fmt.Errorf("failed to get worktree of git repository: %w", err)
		}
		if _, err := worktree.Add("."); err != nil {
			return fmt.Errorf("failed to add files to git repository: %w", err)
		}
		if _, err := worktree.Commit("feat: initial repository with main templates", &git.CommitOptions{}); err != nil {
			return fmt.Errorf("failed to commit main files to git repository: %w", err)
		}
		return nil
	}); err != nil {
		return err
	}
	if err := spinner.Run("Download templates", func() error {
		addSubModuleCmd := exec.Command("git", "submodule", "add", settings.TemplateRepositoryUrl, "templates")
		addSubModuleCmd.Dir = settings.Directory
		if err := addSubModuleCmd.Run(); err != nil {
			return fmt.Errorf("failed to add submodule to git repository: %w", err)
		}
		repo, err := git.PlainOpen(settings.Directory)
		if err != nil {
			return fmt.Errorf("failed to open git repository: %w", err)
		}
		worktree, err := repo.Worktree()
		if err != nil {
			return fmt.Errorf("failed to get worktree of git repository: %w", err)
		}
		if _, err := worktree.Add("."); err != nil {
			return fmt.Errorf("failed to add files to git repository: %w", err)
		}
		if _, err := worktree.Commit("feat: add templates submodule", &git.CommitOptions{}); err != nil {
			return fmt.Errorf("failed to commit template files to git repository: %w", err)
		}
		return nil
	}); err != nil {
		return err
	}
	if settings.IncludeTestlib {
		if err := spinner.Run("Download testlib", func() error {
			addSubModuleCmd := exec.Command("git", "submodule", "add", "git@github.com:MikeMirzayanov/testlib.git", "testlib")
			addSubModuleCmd.Dir = settings.Directory
			if err := addSubModuleCmd.Run(); err != nil {
				return fmt.Errorf("failed to add submodule to git repository: %w", err)
			}
			if err := os.Symlink(filepath.Join("testlib", "testlib.h"), filepath.Join(settings.Directory, "testlib.h")); err != nil {
				return fmt.Errorf("failed to create symlink to testlib.h: %w", err)
			}
			repo, err := git.PlainOpen(settings.Directory)
			if err != nil {
				return fmt.Errorf("failed to open git repository: %w", err)
			}
			worktree, err := repo.Worktree()
			if err != nil {
				return fmt.Errorf("failed to get worktree of git repository: %w", err)
			}
			if _, err := worktree.Add("."); err != nil {
				return fmt.Errorf("failed to add files to git repository: %w", err)
			}
			if _, err := worktree.Commit("feat: add testlib submodule", &git.CommitOptions{}); err != nil {
				return fmt.Errorf("failed to commit template files to git repository: %w", err)
			}
			return nil
		}); err != nil {
			return err
		}
	}
	return nil
}
