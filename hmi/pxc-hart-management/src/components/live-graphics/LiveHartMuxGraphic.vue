<template>
<div>
    <h3>Your HART MUX</h3>
    <div class="hart-mux-graphic">
        <div class=io4-grid>
            <div class="hart-mux-graphic-segment" style="grid-column-start: 1; grid-row-start: 3;">
                <img src="@/assets/img/hart_mux_facade.png" />
            </div>
        </div>

        <HartMuxIo4 ioCard=0 />
        <HartMuxIo8 ioCard=1 />
        <HartMuxIo4 ioCard=2 />
    </div>
</div>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import HartMuxIo4 from '@/components/live-graphics/HartMuxIo4.vue'
import HartMuxIo8 from '@/components/live-graphics/HartMuxIo8.vue'
import { hartServerUrl, HartMuxDto } from '../../types'

@Component({
    components: {
        HartMuxIo4,
        HartMuxIo8
    }
})
export default class LiveHartMuxGraphic extends Vue {
    private ioCardSizes: number[] = []

    mounted () {
        
    }

    scanIoCards () {
        this.$http.get(hartServerUrl + '/info').then(res => {
            const data = res.data as HartMuxDto
            const deviceData = data.devices
            deviceData.forEach(d => {
                
            })
        })
    }
}
</script>