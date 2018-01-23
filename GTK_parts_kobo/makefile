all: GTK_parts.zip

KoboRoot.tgz:
	cd KoboRoot && tar -czf ../KoboRoot.tgz .

GTK_parts.zip: KoboRoot.tgz
	zip GTK_parts.zip -r kbmenupngs eView KoboRoot.tgz

clean:
	-rm -f GTK_parts.zip KoboRoot.tgz
