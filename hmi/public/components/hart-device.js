define([
    'knockout',
    'exports',
    'text!../components/hart-device.html'
], function (ko, hartDevice, template) {

    function HartDevice(params) {
        this.name = "Generic HART device"
    }

    hartDevice.viewModel = HartDevice; 
    hartDevice.template = template;

    ko.components.register('hart-device', hartDevice);

});
  