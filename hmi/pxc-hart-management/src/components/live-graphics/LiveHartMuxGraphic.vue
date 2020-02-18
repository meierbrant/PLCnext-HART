<template>
<div>
    <h3>Your HART MUX</h3>
    <div class="hart-mux-graphic">
        <div class=io4-grid>
            <div class="hart-mux-graphic-segment" style="grid-column-start: 1; grid-row-start: 3;">
                <img src="@/assets/img/hart_mux_facade.png" />
            </div>
        </div>

        <HartMuxIoCard v-for="(mod, i) in gw.modules" v-bind:key="i" :gw="gw" :ioCard="i" :modType="mod" :deviceData="deviceData"/>
    </div>
</div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import HartMuxIoCard from '@/components/live-graphics/HartMuxIoCard.vue'
import { hartServerUrl, HartMuxDto, HartGw, HartGwDto, HartDeviceDto } from '../../types'

@Component({
    components: {
        HartMuxIoCard
    }
})
export default class LiveHartMuxGraphic extends Vue {
    @Prop() gwSN: number
    public gw: HartGw = {ip: "", modules: [], serialNo: 0}
    public deviceData: HartDeviceDto[] = []
    private ioCardSizes: number[] = []
    private polling: number = 0

    mounted () {
        this.gwLookup()
        this.updateDeviceData()
        this.polling = setInterval(this.updateDeviceData, 2000)
    }

    scanIoCards () {
        this.$http.get(hartServerUrl + '/' + this.gwSN + '/info').then(res => {
            const data = res.data as HartMuxDto
            const deviceData = data.devices
            deviceData.forEach(d => {
                
            })
        })
    }

    gwLookup () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            const gws = res.data as HartGwDto
            this.gw = gws.gateways.find(gw => gw.serialNo == this.gwSN) || this.gw
        })
    }

    updateDeviceData () {
        this.$http.get(hartServerUrl + '/gw/' + this.gw.serialNo + '/info').then(res => {
            const data = res.data as HartMuxDto
            this.deviceData = data.devices
        })
    }
}
</script>