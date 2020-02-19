define([
    'knockout',
    'exports',
    'text!../components/hart-mux-graphic-device.html'
], function (ko, component, template) {

    function HartMuxGraphicDevice(params) {
        this.device = params.device;
        this.status = ko.computed(() => {
            if (this.device() !== undefined) {
                return "online";
            } else {
                return "offline";
            }
        });
        this.popoverSettings = {};
        if (this.device() !== undefined) {
            this.popoverSettings.options = {
                title: this.device().longTag,
                content: "IO channel: " + this.device().ioChannel,
                trigger: this.status() == "online" ? 'hover' : ''
            }
        }
    }

    component.viewModel = HartMuxGraphicDevice; 
    component.template = template;

    ko.components.register('hart-mux-graphic-device', component);

});
  