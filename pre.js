function _setColour(func_name, colour)
{
    Module.ccall(func_name, null, ['string'], [colour.replace('#', '')]);
}
function _getArguments()
{
    var survives = document.getElementById('survives').value;
    var born = document.getElementById('born').value;
    var fill = document.getElementById('fill').value;
    return ['-s', survives, '-b', born, '-f', fill];
}
Module['noInitialRun'] = true;
Module['_started'] = false;
Module['quit'] = function ()
{
    Module.ccall('exitLoop');
}
Module['restart'] = function ()
{
    if (Module['_started'])
    {
        Module.quit();
    }
    Module.callMain(_getArguments());
    Module['_started'] = true;
}
Module['_running'] = true;
Module['toggleMainLoop'] = function ()
{
    if (Module['_running'])
    {
        Module.pauseMainLoop();
    }
    else
    {
        Module.resumeMainLoop();
    }
    Module['_running'] ^= true;
}