define([
    'knockout',
    'exports',
    'text!../components/hart-device.html'
], function (ko, component, template) {

    function HartDevice(params) {
        this.longTag = "Generic HART device";
        this.cmdRevLvl = "7";
    }

    component.viewModel = HartDevice; 
    component.template = template;

    ko.components.register('hart-device', component);

});
  