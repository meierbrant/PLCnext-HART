define([
    'knockout',
    'exports',
    'text!../components/hart-mux-io8-live-graphic.html'
], function (ko, component, template) {

    function HartMuxIo8LiveGraphic(params) {
        this.deviceData = ko.observableArray([
            {name: "device", ioChannel: 1},
            {name: "device", ioChannel: 2},
            {name: "device", ioChannel: 3},
            {name: "device", ioChannel: 6},
            {name: "device", ioChannel: 8}
        ]);
        
        this.devices = ko.observable({});
        this.deviceNums = [...Array(8).keys()];
        this.updateDevices = deviceData => {
            this.deviceNums.forEach(i => {
                this.devices()[i+1] = deviceData.find(d => d.ioChannel == i+1);
            });
        };
        this.updateDevices(this.deviceData());
    }

    component.viewModel = HartMuxIo8LiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-io8-live-graphic', component);
});
  