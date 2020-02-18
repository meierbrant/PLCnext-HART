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
                    <th>PV</th>
                    <th>SV</th>
                    <th>Last Update</th>
                </tr>
            </thead>
            <tbody>
                <tr class="clickable" v-for="dev in devices" v-bind:key="10*dev.ioCard + dev.channel" v-on:click="navigateToDeviceShow(dev)">
                    <td>{{ dev.longTag }}</td>
                    <td>{{ dev.name }}</td>
                    <td>{{ dev.company }}</td>
                    <td>{{ dev.cmdRevLvl }}</td>
                    <td>{{ displayVar(dev, 'pv') }}</td>
                    <td>{{ displayVar(dev, 'sv') }}</td>
                    <td>{{ dev.vars ? dev.vars.loopCurrent.lastUpdated : '' }}</td>
                </tr>
            </tbody>
        </table>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator';
import { HartDeviceDto, HartMuxDto, hartServerUrl, HartGw, HartGwDto, HartVars, HartDeviceVarsListDto } from '../types'
import { HttpResponse } from 'vue-resource/types/vue_resource';

const dummyVars: HartVars = {
    loopCurrent: {
        units: "mA",
        value: 2,
        lastUpdated: new Date()
    },
    pv: {
        units: "pi's",
        value: 3.141592,
        lastUpdated: new Date()
    },
    sv: {
        units: "<--",
        value: 2.71,
        lastUpdated: new Date()
    }
}

const dummyHartDevice = {
    name: "dummy device",
    company: "Phoenix Contact",
    longTag: "Dummy Long Tag",
    cmdRevLvl: 7,
    manufacturerId: 17,
    privateLabelDistCode: 0,
    profile: 1,
    revision: 1,
    vars: dummyVars
}

interface LiveListCache {
    gw: HartGw,
    devices: HartDeviceDto[]
}

@Component
export default class HartMuxLiveList extends Vue {
    @Prop() gwSN: number
    public gw: HartGw = {ip: "", modules: [], serialNo: 0}
    public devices: HartDeviceDto[] = []
    private polling: number = 0
    private nextIoCardUpdate

    mounted() {
        this.nextIoCardUpdate = 0
        this.polling = setInterval(this.refreshDevices, 2000)

        // pull live list from cache if present
        if (localStorage.liveList) {
            const llc = JSON.parse(localStorage.liveList) as LiveListCache
            if (llc.gw.serialNo === this.gwSN) {
                this.gw = llc.gw
                llc.devices.forEach(dev => {
                    Object.keys(dev.vars).forEach(key => {
                        dev.vars[key].lastUpdated = new Date(dev.vars[key].lastUpdated) // convert the date string back to a Date()
                    })
                })
                this.devices = llc.devices
            }
        } else {
            this.gwLookup()
        }
    }

    beforeDestroy () {
        clearInterval(this.polling)

        // cache live list
        const llc: LiveListCache = {
            gw: this.gw,
            devices: this.devices
        }
        localStorage.liveList = JSON.stringify(llc)
    }

    public refreshDevices() {
        this.$http.get(hartServerUrl + '/gw/' + this.gwSN + '/vars/' + this.nextIoCardUpdate).then((res) => {
            const data = res.data as HartDeviceVarsListDto
            data.devices.forEach(updatedDevice => {
                const thisDevice = (dev) => dev.ioCard == updatedDevice.ioCard && dev.channel == updatedDevice.channel
                const indexOD = this.devices.findIndex(thisDevice)
                Object.keys(updatedDevice.vars).forEach(key => {
                    updatedDevice.vars[key].lastUpdated = new Date()
                })
                if (indexOD >= 0) {
                    const oldDevice = this.devices[indexOD]
                    this.$set(this.devices, indexOD, updatedDevice)
                } else {
                    this.devices.push(updatedDevice)
                }
            })
        })
        this.nextIoCardUpdate = (this.nextIoCardUpdate + 1) % this.gw.modules.length;
    }

    public refreshDevicesNoVars(): PromiseLike<void | HttpResponse> {
        return this.$http.get(hartServerUrl + '/gw/' + this.gwSN + '/info').then((res) => {
            const data = res.data as HartDeviceVarsListDto
            let list: HartDeviceDto[] = []
            data.devices.forEach(device => {
                list.push(device)
            })
            this.devices = list
        })
    }

    public gwLookup () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            const gws = res.data as HartGwDto
            this.gw = gws.gateways.find(gw => gw.serialNo === this.gwSN) || this.gw
        })
    }

    public displayVar(dev: HartDeviceDto, variable: string): string {
        return (dev.vars && dev.vars[variable] && dev.vars[variable].value) ? (dev.vars[variable].value.toFixed(2) + " " + dev.vars[variable].units) : ''
    }

    public navigateToDeviceShow(dev: HartDeviceDto) {
        this.$router.push({ path: this.deviceUrl(dev) })
    }

    private deviceUrl(dev: HartDeviceDto): string {
        return '/gateways/' + this.gwSN + '/device/' + dev.ioCard + '/' + dev.channel
    }
}
</script>