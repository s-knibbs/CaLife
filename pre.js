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
Module['quit'] = function ()
{
    Module.ccall('exitLoop');
}
Module['restart'] = function ()
{
    Module.quit();
    arguments = _getArguments();
    Module.callMain(arguments);
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