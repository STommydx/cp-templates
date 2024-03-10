package repoinit

import (
	"embed"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path"

	"github.com/STommydx/cp-templates/tools/ccli/spinner"
	"github.com/go-git/go-git/v5"
)

type Settings struct {
	Directory             string
	NumberOfMainPrograms  int
	TemplateRepositoryUrl string
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
		for i := 1; i <= settings.NumberOfMainPrograms; i++ {
			if err := func() error {
				templateFile, err := templatesFS.Open("templates/main.cpp")
				if err != nil {
					return fmt.Errorf("failed to open template file: %w", err)
				}
				defer templateFile.Close()
				filename := fmt.Sprintf("%c.cpp", rune(64+i))
				file, err := os.Create(path.Join(settings.Directory, filename))
				if err != nil {
					return fmt.Errorf("failed to create file: %w", err)
				}
				defer file.Close()
				if _, err := io.Copy(file, templateFile); err != nil {
					return fmt.Errorf("failed to copy template file: %w", err)
				}
				return nil
			}(); err != nil {
				return err
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
	return nil
}
