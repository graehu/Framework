//modified version of: https://gist.github.com/AndrewBarfield/3ee59402501422cdec47
// #todo: should this be a module?
var util = util || {};
util.toArray = function(list) {
    return Array.prototype.slice.call(list || [], 0);
};
var Terminal = Terminal || function(cmdLineContainer, outputContainer) {
    window.URL = window.URL || window.webkitURL;
    window.requestFileSystem = window.requestFileSystem || window.webkitRequestFileSystem;
    var cmdLine_ = document.querySelector(cmdLineContainer);
    var output_ = document.querySelector(outputContainer);

    const CMDS_ = [
	'clear', 'date', 'echo', 'help', 'uname', 'params', 'logs'
    ];
    
    var fs_ = null;
    var cwd_ = null;
    var history_ = [];
    var histpos_ = 0;
    var histtemp_ = 0;

    output_.addEventListener('click', function(e) {
	cmdLine_.focus();
    }, false);
    cmdLine_.addEventListener('click', inputTextClick_, false);
    cmdLine_.addEventListener('keydown', historyHandler_, false);
    cmdLine_.addEventListener('keydown', processNewCommand_, false);

    //
    function inputTextClick_(e) {
	this.value = this.value;
    }

    //
    function historyHandler_(e) {
	if (history_.length) {
	    if (e.keyCode == 38 || e.keyCode == 40) {
		if (history_[histpos_]) {
		    history_[histpos_] = this.value;
		} else {
		    histtemp_ = this.value;
		}
	    }

	    if (e.keyCode == 38) { // up
		histpos_--;
		if (histpos_ < 0) {
		    histpos_ = 0;
		}
	    } else if (e.keyCode == 40) { // down
		histpos_++;
		if (histpos_ > history_.length) {
		    histpos_ = history_.length;
		}
	    }

	    if (e.keyCode == 38 || e.keyCode == 40) {
		this.value = history_[histpos_] ? history_[histpos_] : histtemp_;
		this.value = this.value; // Sets cursor to end of input.
	    }
	}
    }

    //
    function processNewCommand_(e) {
	if (e.keyCode == 9) { // tab
	    e.preventDefault();
	    // Implement tab suggest.
	} else if (e.keyCode == 13) { // enter
	    // Save shell history.
	    if(this.value.length == 0)
	    {
		return;
	    }
	    if (this.value) {
		history_[history_.length] = this.value;
		histpos_ = history_.length;
	    }

	    // Duplicate current input and append to output section.
	    var line = this.parentNode.parentNode.cloneNode(true);
	    line.removeAttribute('id')
	    line.classList.add('line');
	    var input = line.querySelector('input.cmdline');
	    input.autofocus = false;
	    input.readOnly = true;
	    output_.appendChild(line);

	    if (this.value && this.value.trim()) {
		var args = this.value.split(' ').filter(function(val, i) {
		    return val;
		});
		var cmd = args[0].toLowerCase();
		args = args.splice(1); // Remove cmd from arg list.
	    }

	    switch (cmd) {
            case 'clear':
		output_.innerHTML = '';
		this.value = '';
		output('<h2 style="letter-spacing: 2px">Terminal</h2><p>Enter "help" for more information.</p>');
		return;
            case 'date':
		output( new Date() );
		break;
            case 'echo':
		output( " "+args.join(' ') );
		break;
            case 'help':
		output('<div class="ls-files">'+ CMDS_.join('<br>') + '</div>');
		break;
            case 'uname':
		output(navigator.appVersion);
		break;
	    case 'params':
		if(args.length != 0)
		{
		    var post = new XMLHttpRequest();
		    post.open('POST', window.location.hostname+"/params", true);
		    post.send(args.join(' '));
		    output("sent: "+window.location.hostname+"/params "+args.join(' '))
		}
		else
		{
		    output("no arguments supplied.");
		}
		break;
	    case 'logs':
		console.alllogs.forEach(function(item, index){output(item);});
		break;
            default:
		if (cmd) {
		    output(cmd + ': command not found');
		}
	    };
	    this.value = ''; // Clear/setup line for next input.
	}
    }

    //
    function output(html) {
	output_.insertAdjacentHTML('beforeEnd', '<p>' + html + '</p>');
    }

    // Cross-browser impl to get document's height.
    function getDocHeight_() {
	var d = document;
	return Math.max(
            Math.max(d.body.scrollHeight, d.documentElement.scrollHeight),
            Math.max(d.body.offsetHeight, d.documentElement.offsetHeight),
            Math.max(d.body.clientHeight, d.documentElement.clientHeight)
	);
    }
    function HookLogs()
    {
	console.defaultLog = console.log.bind(console);
	console.logs = [];
	console.alllogs = [];
	console.onhookupdated = function(){};
	console.log = function()
	{
	    // default &  console.log()
	    console.defaultLog.apply(console, arguments);
	    // new & array data
	    console.logs.push(Array.from(arguments));
	    console.alllogs.push(Array.from(arguments));
	    console.onhookupdated();
	}
	console.defaultError = console.error.bind(console);
	console.errors = [];
	console.error = function()
	{
	    // default &  console.error()
	    console.defaultError.apply(console, arguments);
	    // new & array data
	    console.errors.push(Array.from(arguments));
	    console.alllogs.push(Array.from(arguments));
	    console.onhookupdated();
	}
	console.defaultWarn = console.warn.bind(console);
	console.warns = [];
	console.warn = function()
	{
	    // default &  console.warn()
	    console.defaultWarn.apply(console, arguments);
	    // new & array data
	    console.warns.push(Array.from(arguments));
	    console.alllogs.push(Array.from(arguments));
	    console.onhookupdated();
	}
	console.defaultDebug = console.debug.bind(console);
	console.debugs = [];
	console.debug = function()
	{
	    // default &  console.debug()
	    console.defaultDebug.apply(console, arguments);
	    // new & array data
	    console.debugs.push(Array.from(arguments));
	    console.alllogs.push(Array.from(arguments));
	    console.onhookupdated();
	}
	console.log("----logs hooked----");
    }
    HookLogs();
    //
    return {
	init: function() {
	    output('<h2 style="letter-spacing: 2px">Terminal</h2><p>Enter "help" for more information.</p>');
	},
	output: output
    }
    
};
