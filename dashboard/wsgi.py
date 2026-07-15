from smartagri.remote_app import create_configured_publisher, create_remote_app

publisher = create_configured_publisher()
app = create_remote_app({"PUBLISHER": publisher})
