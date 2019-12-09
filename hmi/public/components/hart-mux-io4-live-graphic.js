define([
    'knockout',
    'exports',
    'text!../components/hart-mux-io4-live-graphic.html'
], function (ko, component, template) {

    function HartMuxIo4LiveGraphic(params) {
        this.devices = ko.observable([
            {name: "device", ioChannel: 1},
            {name: "device", ioChannel: 2},
            {name: "device", ioChannel: 4}
        ]);
        if ($('.io4-grid.add-facade').has('div.hart-facade').length == 0) {
            $('.io4-grid.add-facade').append("<div class=\"hart-facade\" style=\"grid-row-start: 3;\">\
            <img src=\"/img/hart_mux_io4_facade.png\" />\
            </div>");
        }
    }

    component.viewModel = HartMuxIo4LiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-io4-live-graphic', component);

});
  