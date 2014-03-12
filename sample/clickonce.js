function getIeVersion()
{
    var undef,rv = -1; // Return value assumes failure.
    var ua = window.navigator.userAgent;
    var msie = ua.indexOf('MSIE ');
    var trident = ua.indexOf('Trident/');

    if (msie > 0) {
        // IE 10 or older => return version number
        rv = parseInt(ua.substring(msie + 5, ua.indexOf('.', msie)), 10);
    } else if (trident > 0) {
        // IE 11 (or newer) => return version number
        var rvNum = ua.indexOf('rv:');
        rv = parseInt(ua.substring(rvNum + 3, ua.indexOf('.', rvNum)), 10);
    }

    return ((rv > -1) ? rv : undef);
}

function checkClickOnce(specialUa, requiredClrVer, callbackFunc)
{
	var onWindows = (window.navigator.platform.indexOf('Win') === 0);
	if (!onWindows)
	{
		var callBackAttr = { code: 'NOTWIN' };
		callbackFunc(callBackAttr);
		return;
	}

	var dotNetVersion = 0;
	var hasClickOnce = false;

	var ua = window.navigator.userAgent; //newer IE actually don't expose .net versions in HTTP userAgent, just here
	if (specialUa != null) //the Firefox ClickOnce plugin was installed when the page was requested
	{
		ua = specialUa;
		hasClickOnce = true;
	}

	var ieVersion = getIeVersion();
	var isIe = (ieVersion > 0);
	if (!isIe && !hasClickOnce)
	{
		//we need to find out if there's a plugin handling ClickOnce on other browsers
		var mimeHandler = window.navigator.mimeTypes["application/x-ms-application"];
		if (mimeHandler != null && mimeHandler.enabledPlugin != null) //ClickOnce via plugin
		{
			hasClickOnce = true;

			var embed = document.createElement('embed');
			embed.setAttribute('type', 'application/x-ms-application');
			embed.setAttribute('width', 0); embed.setAttribute('height', 0);
			document.body.appendChild(embed); //plugins dont usually instantiate untill they are in the doc

			if (typeof (embed.clrVersion) == 'string') //this plugin supports fetching .net versions
			{
				specialUa = embed.clrVersion;
				ua = specialUa;
			}

			if (embed.parentNode) { embed.parentNode.removeChild(embed); }
		}
	}

	if (ua.indexOf('.NET4') >= 0)
	{
		dotNetVersion = 4;
	}
	else if (ua.indexOf('.NET CLR 3.5') >= 0)
	{
		dotNetVersion = 3.5;
	}
	else if (ua.indexOf('.NET CLR 3') >= 0)
	{
		dotNetVersion = 3;
	}
	else if (ua.indexOf('.NET CLR 2') >= 0)
	{
		dotNetVersion = 2;
	}
	else if (ua.indexOf('.NET CLR 1.1') >= 0)
	{
		dotNetVersion = 1.1;
	}
	else if (ua.indexOf('.NET CLR 1') >= 0)
	{
		dotNetVersion = 1;
	}
	
	if (isIe)
	{
		if (dotNetVersion >= 2) //all IE after .net 2 has ClickOnce support
		{
			hasClickOnce = true;
		}
		else
		{
			hasClickOnce = false;
		}
	}

	//if we didn't find any .net, we can't trust the check unless we're on IE (or specialUa)
	var skipDotNetCheck = false;
	if (isIe)
	{
		skipDotNetCheck = false;
	}
	else if (dotNetVersion == 0)
	{
		if (specialUa == null)
		{
			skipDotNetCheck = true;
		}
		else //we have specialUa, so we know exactly what CLRs are installed
		{
			skipDotNetCheck = false;
		}
	}

	var callBackAttr = { clrSkipped: skipDotNetCheck, clrVersion: dotNetVersion, clickOnce: hasClickOnce };
	if (!skipDotNetCheck && (dotNetVersion < requiredClrVer))
	{
		callBackAttr["code"] = 'INVCLR';
		callbackFunc(callBackAttr);
		return;
	}

	if (!hasClickOnce)
	{
		callBackAttr["code"] = 'EXTLINK';
		var extLink = callbackFunc(callBackAttr);
		
		if (typeof chrome == 'object' && typeof(chrome.webstore) == 'object') //we can install the chrome extension
		{
			var clickHandlerChrome = function()
			{
				callBackAttr["code"] = 'EXTSTART';
				callbackFunc(callBackAttr);

				chrome.webstore.install("https://chrome.google.com/webstore/detail/mdooolbdbmjaobhdondofgdmnbidlgfh", 
					function()
					{
						callBackAttr["code"] = 'EXTDONE';
						callBackAttr["clickOnce"] = true;
						callbackFunc(callBackAttr);
					},
					function(err)
					{
						callBackAttr["code"] = 'EXTERR';
						callBackAttr["errMsg"] = err;
						var tryAgainLink = callbackFunc(callBackAttr);
						if (tryAgainLink != null)
						{
							tryAgainLink.on("click", clickHandlerChrome);	
						}
					});

				return false;
			};

			extLink.on("click", clickHandlerChrome);
			return;
		}
		else if (typeof (InstallTrigger) == 'object') //we are on Firefox
		{
			extLink.attr('href',"https://addons.mozilla.org/firefox/downloads/latest/9449/platform:5/addon-9449-latest.xpi");
			var clickHandlerFirefox = function()
			{
				callBackAttr["code"] = 'EXTSTART';
				callbackFunc(callBackAttr);
				return true;
			};

			extLink.on("click", clickHandlerFirefox);
			return;
		}
	}

	callBackAttr["code"] = 'DONE';
	return callbackFunc(callBackAttr);
}
