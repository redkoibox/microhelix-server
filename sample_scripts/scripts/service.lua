services["^\\/string\\??(.*)$"] = 
{
	GET =
	{
		consumes = Helix.TEXT_PLAIN,
		produces = Helix.JSON,
		onRequest = Helix.script("string")
	},
	POST =
	{
		consumes = Helix.JSON,
		produces = Helix.JSON,
		onRequest = Helix.script("string")
	},
	DELETE =
	{
		consumes = Helix.JSON,
		produces = Helix.JSON,
		onRequest = Helix.script("string")
	},
	PUT =
	{
		consumes = Helix.JSON,
		produces = Helix.JSON,
		onRequest = Helix.script("string")
	}
}
