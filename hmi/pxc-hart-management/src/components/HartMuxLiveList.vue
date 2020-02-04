<template>
    <div id="hart-live-list">
        <h3><span data-bind="text: gateway"></span> Live List</h3>
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>Long Tag</th>
                    <th>Name</th>
                    <th>Company</th>
                    <th>Hart Revision</th>
                </tr>
            </thead>
            <tbody>
                <tr v-for="dev in devices" v-bind:key="10*dev.ioCard + dev.channel">
                    <td>{{ dev.longTag }}</td>
                    <td>{{ dev.name }}</td>
                    <td>{{ dev.company }}</td>
                    <td>{{ dev.cmdRevLvl }}</td>
                </tr>
            </tbody>
        </table>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator';
import { HartDeviceDto, HartMuxDto, hartServerUrl, HartGw, HartGwDto } from '../types'

const dummyHartDevice = {
    name: "dummy device",
    company: "Phoenix Contact",
    longTag: "Dummy Long Tag",
    cmdRevLvl: 7,
    manufacturerId: 17,
    privateLabelDistCode: 0,
    profile: 1,
    revision: 1
}

@Component
export default class HartMuxLiveList extends Vue {
    @Prop() gwIp: string
    public gw: HartGw = {ip: "", modules: [], serialNo: 0}
    public devices: HartDeviceDto[] = []
    private polling: number = 0

    mounted() {
        this.gwLookup()
        this.refreshDevices()
        this.polling = setInterval(this.refreshDevices, 2000)
    }

    beforeDestroy () {
        clearInterval(this.polling)
    }

    public refreshDevices() {
        this.$http.get(hartServerUrl + '/gw/' + this.gw.serialNo + '/info').then((res) => {
            const data = res.data as HartMuxDto
            let list: HartDeviceDto[] = []
            let id = 1
            data.devices.forEach(device => {
                list.push(device)
            })
            this.devices = list
        })
    }

    gwLookup () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            const gws = res.data as HartGwDto
            this.gw = gws.gateways.find(gw => gw.ip === this.gwIp) || this.gw
        })
    }
}
</script>