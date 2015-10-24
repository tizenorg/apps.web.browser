try {
	function GetRSSLinks() {
		var links = document.getElementsByTagName('link');
		var rsslinks=[];
		var result="";

		for (var i = 0; i < links.length; ++i) {
			if (links[i].getAttribute('type') == 'application/rss+xml') {
				if (links[i] && links[i].hasAttribute('href') && links[i].hasAttribute('title')) {
					link_info = links[i].getAttribute('title') + "|" + links[i].getAttribute('href');
					rsslinks.push(link_info);
				}
				else if (links[i] && links[i].hasAttribute('href'))
					rsslinks.push(links[i].getAttribute('href'));
			}
		}
		rsslinks.join(',');
		result=rsslinks.toString();
		return result;
	}
	GetRSSLinks();
}catch(e) { }
