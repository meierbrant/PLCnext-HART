<template>
<div>
    <h3>Your HART MUX</h3>
    <div class="hart-mux-graphic">
        <div class=io4-grid>
            <div class="hart-mux-graphic-segment" style="grid-column-start: 1; grid-row-start: 3;">
                <img src="@/assets/img/hart_mux_facade.png" />
            </div>
        </div>

        <HartMuxIo4 />
    </div>
</div>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import HartMuxIo4 from '@/components/live-graphics/HartMuxIo4.vue'

@Component({
    components: {
        HartMuxIo4
    }
})
export default class LiveHartMuxGraphic extends Vue {
    
}
</script>