{
  pkgs,
  buildGoApplication ? pkgs.buildGoApplication,
}:

buildGoApplication {
  pname = "ccli";
  version = "1.11.0";
  src = ./.;
  pwd = ./.;
  modules = ./gomod2nix.toml;
}
