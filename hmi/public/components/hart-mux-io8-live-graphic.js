define([
    'knockout',
    'exports',
    'text!../components/hart-mux-io8-live-graphic.html'
], function (ko, component, template) {

    function HartMuxIo8LiveGraphic(params) {
        this.longTag = "Generic HART device";
        this.cmdRevLvl = "7";
    }

    component.viewModel = HartMuxIo8LiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-io8-live-graphic', component);

});
  