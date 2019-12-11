define([
    'knockout',
    'exports',
    'text!../components/hart-mux-io4-live-graphic.html'
], function (ko, component, template) {

    function HartMuxIo4LiveGraphic(params) {
        this.deviceData = ko.observable([
            {name: "device", ioChannel: 1},
            {name: "device", ioChannel: 2},
            {name: "device", ioChannel: 4}
        ]);
        
        this.devices = ko.observable({});
        this.deviceNums = [...Array(4).keys()];
        this.updateDevices = deviceData => {
            this.deviceNums.forEach(i => {
                this.devices()[i+1] = deviceData.find(d => d.ioChannel == i+1);
            });
        };
        this.updateDevices(this.deviceData());
    }

    component.viewModel = HartMuxIo4LiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-io4-live-graphic', component);

});
  