define([
    'knockout',
    'exports',
    'text!../components/hart-device.html'
], function (ko, component, template) {

    function HartDevice(params) {
        this.longTag = "Generic HART device";
        this.cmdRevLvl = "7";
        this.cmd = 0;
        this.cmdResponse = {
            data: "Result of command goes here..."
        };

        this.sendCmd = function(cmd) {
            console.log("send command " + cmd);
            const result = {
                data: "You sent command " + cmd
            };
            this.cmdResponse = result;
        };
    }

    component.viewModel = HartDevice; 
    component.template = template;

    ko.components.register('hart-device', component);

});
  