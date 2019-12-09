define([
    'knockout',
    'exports',
    'text!../components/hart-mux-graphic-device.html'
], function (ko, component, template) {

    function HartMuxGraphicDevice(params) {
        this.name = params.device.name;
        this.ioChannel = params.device.ioChannel;
    }

    component.viewModel = HartMuxGraphicDevice; 
    component.template = template;

    ko.components.register('hart-mux-graphic-device', component);

});
  