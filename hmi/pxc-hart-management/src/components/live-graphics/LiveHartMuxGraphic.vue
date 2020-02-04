<template>
<div>
    <h3>Your HART MUX</h3>
    <div class="hart-mux-graphic">
        <div class=io4-grid>
            <div class="hart-mux-graphic-segment" style="grid-column-start: 1; grid-row-start: 3;">
                <img src="@/assets/img/hart_mux_facade.png" />
            </div>
        </div>

        <HartMuxIoCard v-for="(mod, i) in gw.modules" v-bind:key="i" :gw="gw" :ioCard="i" :modType="mod" />
    </div>
</div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import HartMuxIoCard from '@/components/live-graphics/HartMuxIoCard.vue'
import { hartServerUrl, HartMuxDto, HartGw, HartGwDto } from '../../types'

@Component({
    components: {
        HartMuxIoCard
    }
})
export default class LiveHartMuxGraphic extends Vue {
    @Prop() gwIp: string
    public gw: HartGw = {ip: "", modules: [], serialNo: 0}
    private ioCardSizes: number[] = []

    mounted () {
        this.gwLookup()
    }

    scanIoCards () {
        this.$http.get(hartServerUrl + '/' + this.gw.serialNo + '/info').then(res => {
            const data = res.data as HartMuxDto
            const deviceData = data.devices
            deviceData.forEach(d => {
                
            })
        })
    }

    gwLookup () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            const gws = res.data as HartGwDto
            this.gw = gws.gateways.find(gw => gw.ip == this.gwIp) || this.gw
        })
    }
}
</script>