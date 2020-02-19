define([
    'knockout',
    'exports',
    'text!../components/hart-mux-live-graphic.html'
], function (ko, component, template) {

    function HartMuxLiveGraphic(params) {
        this.longTag = "Generic HART device";
        this.cmdRevLvl = "7";
    }

    component.viewModel = HartMuxLiveGraphic; 
    component.template = template;

    ko.components.register('hart-mux-live-graphic', component);

});
  