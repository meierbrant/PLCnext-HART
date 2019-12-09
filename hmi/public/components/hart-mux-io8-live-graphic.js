define([
    'knockout',
    'exports',
    'text!../components/hart-mux-io8-live-graphic.html'
], function (ko, component, template) {

    function HartMuxIo8LiveGraphic(params) {
        // this.devices = ko.observable([
        //     {name: "device", ioChannel: 1},
        //     {name: "device", ioChannel: 2},
        //     {name: "device", ioChannel: 3},
        //     {name: "device", ioChannel: 6},
        //     {name: "device", ioChannel: 8}
        // ]);
        this.devices = ko.observableArray([
        ]);
        console.log("once");
        console.log($('.io8-grid.add-facade'));
        $('.io8-grid.add-facade').append("<div class=\"hart-facade\">\
            <img src=\"/img/hart_mux_io8_facade.png\" />\
            </div>").removeClass('add-facade');
        this.devices.push({name: "device", ioChannel: 1});
        this.devices.push({name: "device", ioChannel: 2});
        this.devices.push({name: "device", ioChannel: 3});
        console.log(this.devices());
    }

    component.viewModel = HartMuxIo8LiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-io8-live-graphic', component);
});
  