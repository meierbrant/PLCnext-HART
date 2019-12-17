define([
    'knockout',
    'exports',
    'text!../components/hart-live-device-list.html'
], function (ko, component, template) {

    function HartLiveDeviceList(params) {
        let self = this;
        self.gateway = "HART MUX";
        self.devices = ko.observable([]);
        self.getDevices = function() {
            $.get("http://localhost:5900/info", {}, function(data) {
                self.devices(data.devices);
                console.log(self.devices());
            });
        }
        self.getDevices(); // initially load the list
        setInterval(self.getDevices, 2000); // update the list every 2 seconds
    }

    component.viewModel = HartLiveDeviceList; 
    component.template = template;

    ko.components.register('hart-live-device-list', component);
    //ko.applyBindings(self);
});
  