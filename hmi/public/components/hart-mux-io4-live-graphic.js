define([
    'knockout',
    'exports',
    'text!../components/hart-mux-io4-live-graphic.html'
], function (ko, component, template) {

    function HartMuxIo4LiveGraphic(params) {
        this.longTag = "Generic HART device";
        this.cmdRevLvl = "7";
    }

    component.viewModel = HartMuxIo4LiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-io4-live-graphic', component);

});
  