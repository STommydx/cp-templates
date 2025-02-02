{ ... }:

{
  projectRootFile = "flake.nix";

  programs.clang-format.enable = true;
  programs.gofmt.enable = true;
  programs.mdformat = {
    enable = true;
    settings.number = true;
  };
  programs.nixfmt.enable = true;
  programs.yamlfmt.enable = true;
}
