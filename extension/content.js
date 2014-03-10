
// Set properties on the DOM so page can tell we're here
var script = document.createElement("script");
script.type = "text/javascript";
script.innerText = "document.isClickOnceSupported = true;";   
document.documentElement.insertBefore(script);

document.onclick = function(e)
{
    e = e || window.event;
    var element = e.target || e.srcElement;

    if (element.tagName == 'A' && element.href.match(/\.application$/))
    {
        var embed = document.createElement('embed');
        embed.setAttribute('type', 'application/x-ms-application');
        embed.launchClickOnce(element.href);
        return false;
    }
}


