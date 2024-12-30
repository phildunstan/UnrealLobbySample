# Unreal Lobby Sample

This project contains sample code and configuration for hosting and joining a network lobby in Unreal.

It contains custom configurations for:
* Online Subsystem Null
* Online Subsystem EOS
* Online Subsystem Steam
* Online Services Null
* Online Services Epic
* Online Services Steam (via the OSS Adapter plugin)

Testing a specific configuration is managed by using the -customconfig command line when running the game.

To test Online Subsystem EOS or Online Services EPIC, it is necessary to modify the Engine.ini config files in the respective Config/Custom directory to specify client specific IDs. 
