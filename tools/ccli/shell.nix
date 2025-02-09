{
  pkgs,
  mkGoEnv ? pkgs.mkGoEnv,
  gomod2nix ? pkgs.gomod2nix,
}:

let
  goEnv = mkGoEnv { pwd = ./.; };
in
pkgs.mkShell {
  packages =
    [
      goEnv
      gomod2nix
    ]
    ++ (with pkgs; [
      air
      cobra-cli
    ]);
}
