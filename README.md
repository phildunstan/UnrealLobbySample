# Unreal Lobby Sample

This project contains sample code and configuration for hosting and joining a network lobby in Unreal.

It contains custom configurations for:
* Online Subsystem Null
* Online Subsystem EOS
* Online Subsystem Steam
* Online Services Null
* Online Services Epic
* Online Services Steam (via the OSS Adapter plugin)

Testing a specific configuration is done by modifying the LobbySample.Build.cs to set the OnlineConfig to the config that you want to test and running the game with -customconfig <ConfigType>.

To test Online Subsystem EOS or Online Services EPIC, it is necessary to modify the Engine.ini config files in the respective Config/Custom directory to specify client specific IDs.

# Build dependencies

This project depends on VesCodes ImGui plugin - https://github.com/VesCodes/ImGui. This plugin is not distributed with this project.

